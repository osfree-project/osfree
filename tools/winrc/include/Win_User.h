/*  Win_User.h	1.55   SDK Structure definitions, and API's.
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/

#ifndef Win_User__h
#define Win_User__h

/* User typedefs ***************************************************/

DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HICON);

typedef HICON HCURSOR;      /* HICONs & HCURSORs are polymorphic */


#ifndef NOAPIPROTO  /* Prototypes */

UINT    WINAPI GetDoubleClickTime(void);
void    WINAPI SetDoubleClickTime(UINT);

#endif              /* NOAPIPROTO */


#define WM_DEVMODECHANGE    0x001B
#define WM_TIMECHANGE       0x001E

/* System metrics **************************************************/
#ifndef NOSYSMETRICS

#ifndef NOAPIPROTO  /* Prototypes */

int WINAPI GetSystemMetrics(int);

#endif              /* NOAPIPROTO */

/* GetSystemMetrics() codes */
#define SM_CXSCREEN         0
#define SM_CYSCREEN         1
#define SM_CXVSCROLL        2
#define SM_CYHSCROLL        3
#define SM_CYCAPTION        4
#define SM_CXBORDER         5
#define SM_CYBORDER         6   
#define SM_CXDLGFRAME       7
#define SM_CYDLGFRAME       8
#define SM_CYVTHUMB         9
#define SM_CXHTHUMB         10
#define SM_CXICON           11
#define SM_CYICON           12
#define SM_CXCURSOR         13
#define SM_CYCURSOR         14
#define SM_CYMENU           15
#define SM_CXFULLSCREEN     16
#define SM_CYFULLSCREEN     17
#define SM_CYKANJIWINDOW    18
#define SM_MOUSEPRESENT     19
#define SM_CYVSCROLL        20
#define SM_CXHSCROLL        21
#define SM_DEBUG            22
#define SM_SWAPBUTTON       23
#define SM_RESERVED1        24
#define SM_RESERVED2        25
#define SM_RESERVED3        26
#define SM_RESERVED4        27
#define SM_CXMIN            28
#define SM_CYMIN            29
#define SM_CXSIZE           30
#define SM_CYSIZE           31
#define SM_CXFRAME          32
#define SM_CYFRAME          33
#define SM_CXMINTRACK       34
#define SM_CYMINTRACK       35
#define SM_CXDOUBLECLK      36
#define SM_CYDOUBLECLK      37
#define SM_CXICONSPACING    38
#define SM_CYICONSPACING    39
#define SM_MENUDROPALIGNMENT    40
#define SM_PENWINDOWS       41
#define SM_DBCSENABLED      42
#define SM_CMOUSEBUTTONS    43

	/* Win95 name change */
#define SM_CXFIXEDFRAME		SM_CXDLGFRAME
#define SM_CYFIXEDFRAME		SM_CYDLGFRAME
#define SM_CXSIZEFRAME		SM_CXFRAME
#define SM_CYSIZEFRAME		SM_CYFRAME

	/* Win95 */
#define SM_SECURE		44
#define SM_CXEDGE		45
#define SM_CYEDGE		46
#define SM_CXMINSPACING		47
#define SM_CYMINSPACING		48
#define SM_CXSMICON		49
#define SM_CYSMICON		50
#define SM_CYSMCAPTION		51
#define SM_CXSMSIZE		52
#define SM_CYSMSIZE		53
#define SM_CXMENUSIZE		54
#define SM_CYMENUSIZE		55
#define SM_ARRANGE		56
#define SM_CXMINIMIZED		57
#define SM_CYMINIMIZED		58
#define SM_CXMAXTRACK		59
#define SM_CYMAXTRACK		60
#define SM_CXMAXIMIZED		61
#define SM_CYMAXIMIZED		62
#define SM_NETWORK		63
#define SM_CLEANBOOT		64
#define SM_CXDRAG		68
#define SM_CYDRAG		69

	/* all versions */
#define SM_SHOWSOUNDS		70

	/* Win95 */
#define SM_CXMENUCHECK		71  /* replaces GetMenuCheckMarkDimensions() */
#define SM_CYMENUCHECK		72  /* replaces GetMenuCheckMarkDimensions() */
#define SM_SLOWMACHINE		73
#define SM_MIDEASTENABLED	74

	/* all versions */
#define SM_CMETRICS		75

/* SM_ARRANGE return codes */
#define ARW_BOTTOMLEFT		0x0000L
#define ARW_BOTTOMRIGHT		0x0001L
#define ARW_TOPLEFT		0x0002L
#define ARW_TOPRIGHT		0x0003L
#define ARW_STARTMASK		0x0003L
#define ARW_STARTRIGHT		0x0001L
#define ARW_STARTTOP		0x0002L

#define ARW_LEFT		0x0000L
#define ARW_RIGHT		0x0000L
#define ARW_UP			0x0004L
#define ARW_DOWN		0x0004L
#define ARW_HIDE		0x0008L
#define ARW_VALID		0x000FL

#endif      /* NOSYSMETRICS */



/* System support **************************************************/

#ifndef NOSYSTEMPARAMSINFO

#ifndef NOAPIPROTO  /* Prototypes */

BOOL    WINAPI SystemParametersInfo(UINT, UINT, VOID FAR*, UINT);

#endif              /* NOAPIPROTO */

#define SPI_GETBEEP                 1
#define SPI_SETBEEP                 2
#define SPI_GETMOUSE                3
#define SPI_SETMOUSE                4
#define SPI_GETBORDER               5
#define SPI_SETBORDER               6
#define SPI_GETKEYBOARDSPEED        10
#define SPI_SETKEYBOARDSPEED        11
#define SPI_LANGDRIVER              12
#define SPI_ICONHORIZONTALSPACING   13
#define SPI_GETSCREENSAVETIMEOUT    14
#define SPI_SETSCREENSAVETIMEOUT    15
#define SPI_GETSCREENSAVEACTIVE     16
#define SPI_SETSCREENSAVEACTIVE     17
#define SPI_GETGRIDGRANULARITY      18
#define SPI_SETGRIDGRANULARITY      19
#define SPI_SETDESKWALLPAPER        20
#define SPI_SETDESKPATTERN          21
#define SPI_GETKEYBOARDDELAY        22
#define SPI_SETKEYBOARDDELAY        23
#define SPI_ICONVERTICALSPACING     24
#define SPI_GETICONTITLEWRAP        25
#define SPI_SETICONTITLEWRAP        26
#define SPI_GETMENUDROPALIGNMENT    27
#define SPI_SETMENUDROPALIGNMENT    28
#define SPI_SETDOUBLECLKWIDTH       29
#define SPI_SETDOUBLECLKHEIGHT      30
#define SPI_GETICONTITLELOGFONT     31
#define SPI_SETDOUBLECLICKTIME      32
#define SPI_SETMOUSEBUTTONSWAP      33
#define SPI_SETICONTITLELOGFONT     34
#define SPI_GETFASTTASKSWITCH       35
#define SPI_SETFASTTASKSWITCH       36
#define SPI_SETWORKAREA             47
#define SPI_GETWORKAREA             48

/* SystemParametersInfo flags */

#define SPIF_UPDATEINIFILE      0x0001
#define SPIF_SENDWININICHANGE   0x0002
#define SPIF_SENDCHANGE		SPIF_SENDWININICHANGE

#endif      /* NOSYSTEMPARAMSINFO */

/* Rectangle support ***********************************************/

#ifndef NOAPIPROTO  /* Prototypes */

void    WINAPI SetRect(RECT FAR*, int, int, int, int);
void    WINAPI SetRectEmpty(RECT FAR*);

void    WINAPI CopyRect(RECT FAR*, const RECT FAR*);

BOOL    WINAPI IsRectEmpty(const RECT FAR*);

BOOL    WINAPI EqualRect(const RECT FAR*, const RECT FAR*);

BOOL    WINAPI IntersectRect(RECT FAR*, const RECT FAR*, const RECT FAR*);
BOOL    WINAPI UnionRect(RECT FAR*, const RECT FAR*, const RECT FAR*);
BOOL    WINAPI SubtractRect(RECT FAR*, const RECT FAR*, const RECT FAR*);

void    WINAPI OffsetRect(RECT FAR*, int, int);
void    WINAPI InflateRect(RECT FAR*, int, int);

BOOL    WINAPI PtInRect(const RECT FAR*, POINT);

#endif              /* NOAPIPROTO */

/* Message support *************************************************/

#ifndef NOAPIPROTO  /* Prototypes */

UINT WINAPI RegisterWindowMessage(LPCSTR);

BOOL    WINAPI PostMessage(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI SendMessage(HWND, UINT, WPARAM, LPARAM);

#endif              /* NOAPIPROTO */

#define WM_NULL         0x0000
#define WM_USER         0x0400      /* Messages below 0x400 are RESERVED */

/* Standard window messages */
/* PenWindows specific messages */
#define WM_PENWINFIRST      0x0380
#define WM_PENWINLAST       0x038F

/* Coalescing messages */
#define WM_COALESCE_FIRST   0x0390
#define WM_COALESCE_LAST    0x039F

#ifndef NOMSG

/* Queued message structure */
typedef struct tagMSG
{
    HWND    hwnd;
    UINT        message;
    WPARAM  wParam;
    LPARAM  lParam;
    DWORD       time;
    POINT   pt;
} MSG;
typedef MSG* PMSG;
typedef MSG NEAR* NPMSG;
typedef MSG FAR* LPMSG;

#ifndef NOAPIPROTO  /* Prototypes */

BOOL    WINAPI GetMessage(MSG FAR*, HWND, UINT, UINT);
BOOL    WINAPI PeekMessage(MSG FAR*, HWND, UINT, UINT, UINT);

void    WINAPI WaitMessage(void);

DWORD   WINAPI GetMessagePos(void);
LONG    WINAPI GetMessageTime(void);

LPARAM  WINAPI GetMessageExtraInfo(void);

BOOL    WINAPI TranslateMessage(const MSG FAR*);
LONG    WINAPI DispatchMessage(const MSG FAR*);

BOOL    WINAPI SetMessageQueue(int);

BOOL    WINAPI GetInputState(void);

DWORD   WINAPI GetQueueStatus(UINT flags);

BOOL    WINAPI PostAppMessage(HTASK, UINT, WPARAM, LPARAM);

void    WINAPI ReplyMessage(LRESULT);
BOOL    WINAPI InSendMessage(void);

BOOL WINAPI CallMsgFilter(MSG FAR*, int);

#endif              /* NOAPIPROTO */

/* PeekMessage() options */
#define PM_NOREMOVE 0x0000
#define PM_REMOVE   0x0001
#define PM_NOYIELD  0x0002

/* GetQueueStatus flags */
#define QS_KEY      0x0001
#define QS_MOUSEMOVE    0x0002
#define QS_MOUSEBUTTON  0x0004
#define QS_MOUSE    (QS_MOUSEMOVE | QS_MOUSEBUTTON)
#define QS_POSTMESSAGE  0x0008
#define QS_TIMER    0x0010
#define QS_PAINT    0x0020
#define QS_SENDMESSAGE  0x0040

#define QS_ALLINPUT     0x007f

/* Special HWND value for use with PostMessage() and SendMessage() */
#define HWND_BROADCAST  ((HWND)0xffff)

#define WH_GETMESSAGE       3

#define WH_CALLWNDPROC      4

#define WH_MSGFILTER        (-1)
#define WH_SYSMSGFILTER     6

/* CallMsgFilter() and WH_SYS/MSGFILTER context codes */
#define MSGF_DIALOGBOX   0
#define MSGF_MENU        2
#define MSGF_MOVE        3
#define MSGF_SIZE        4
#define MSGF_SCROLLBAR   5
#define MSGF_NEXTWINDOW  6
#define MSGF_MAINLOOP    8
#define MSGF_USER        4096

#endif      /* NOMSG */

/* Power management ************************************************/

#define WM_POWER        0x0048

/* wParam for WM_POWER window message and DRV_POWER driver notification */

#define PWR_OK              1
#define PWR_FAIL            (-1)
#define PWR_SUSPENDREQUEST  1
#define PWR_SUSPENDRESUME   2
#define PWR_CRITICALRESUME  3

/* Application termination *****************************************/

#ifndef NOAPIPROTO  /* Prototypes */

void    WINAPI PostQuitMessage(int);

#endif              /* NOAPIPROTO */

#define WM_QUERYENDSESSION  0x0011
#define WM_ENDSESSION       0x0016
#define WM_QUIT             0x0012
#define WM_SYSTEMERROR      0x0017

/* Window class management *****************************************/

typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct tagWNDCLASS
{
    UINT        style;
    WNDPROC lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON   hIcon;
    HCURSOR hCursor;
    HBRUSH  hbrBackground;
    LPCSTR  lpszMenuName;
    LPCSTR  lpszClassName;
} WNDCLASS;
typedef WNDCLASS* PWNDCLASS;
typedef WNDCLASS NEAR* NPWNDCLASS;
typedef WNDCLASS FAR* LPWNDCLASS;


#ifndef NOAPIPROTO  /* Prototypes */

ATOM    WINAPI RegisterClass(const WNDCLASS FAR*);
BOOL    WINAPI UnregisterClass(LPCSTR, HINSTANCE);

BOOL    WINAPI GetClassInfo(HINSTANCE, LPCSTR, WNDCLASS FAR*);
int     WINAPI GetClassName(HWND, LPSTR, int);

#endif              /* NOAPIPROTO */


#ifndef NOWINSTYLES

/* Class styles */
#define CS_VREDRAW      0x0001
#define CS_HREDRAW      0x0002

#define CS_OWNDC        0x0020
#define CS_CLASSDC      0x0040
#define CS_PARENTDC     0x0080

#define CS_SAVEBITS     0x0800

#define CS_DBLCLKS      0x0008

#define CS_BYTEALIGNCLIENT  0x1000
#define CS_BYTEALIGNWINDOW  0x2000

#define CS_NOCLOSE      0x0200

#define CS_KEYCVTWINDOW     0x0004
#define CS_NOKEYCVT     0x0100

#define CS_GLOBALCLASS      0x4000


#endif      /* NOWINSTYLES */



#ifndef NOWINOFFSETS

#ifndef NOAPIPROTO  /* Prototypes */

WORD    WINAPI GetClassWord(HWND, int);
WORD    WINAPI SetClassWord(HWND, int, WORD);
LONG    WINAPI GetClassLong(HWND, int);
LONG    WINAPI SetClassLong(HWND, int, LONG);

#endif              /* NOAPIPROTO */

/* Class field offsets for GetClassLong() and GetClassWord() */
#ifndef	TWIN32
#define GCL_MENUNAME        (-8)
#define GCW_HBRBACKGROUND   (-10)
#define GCW_HCURSOR         (-12)
#define GCW_HICON           (-14)
#define GCW_HMODULE         (-16)
#define GCW_CBWNDEXTRA      (-18)
#define GCW_CBCLSEXTRA      (-20)
#define GCL_WNDPROC         (-24)
#define GCW_STYLE           (-26)
#define GCW_ATOM            (-32)
#else
#define GCL_MENUNAME        (-8)
#define GCL_HBRBACKGROUND   (-10)
#define GCL_HCURSOR         (-12)
#define GCL_HICON           (-14)
#define GCL_HMODULE         (-16)
#define GCL_CBWNDEXTRA      (-18)
#define GCL_CBCLSEXTRA      (-20)
#define GCL_WNDPROC         (-24)
#define GCL_STYLE           (-26)
#define GCW_ATOM            (-32)
#endif


#endif  /* NOWINOFFSETS */



/* Window creation/destruction *************************************/

/* WM_CREATE/WM_NCCREATE lParam struct */
typedef struct tagCREATESTRUCT
{
    void FAR* lpCreateParams;
    HINSTANCE hInstance;
    HMENU     hMenu;
    HWND      hwndParent;
    int       cy;
    int       cx;
    int       y;
    int       x;
    LONG      style;
    LPCSTR    lpszName;
    LPCSTR    lpszClass;
    DWORD     dwExStyle;
} CREATESTRUCT;
typedef CREATESTRUCT FAR* LPCREATESTRUCT;


#ifndef NOAPIPROTO  /* Prototypes */

BOOL    WINAPI IsWindow(HWND);

HWND    WINAPI CreateWindowEx(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, void FAR*);
HWND    WINAPI CreateWindow(LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, void FAR*);

BOOL    WINAPI DestroyWindow(HWND);

HTASK   WINAPI GetWindowTask(HWND);

BOOL    WINAPI IsChild(HWND, HWND);

HWND    WINAPI GetParent(HWND);
HWND    WINAPI SetParent(HWND, HWND);

BOOL    WINAPI IsWindowVisible(HWND);

BOOL    WINAPI ShowWindow(HWND, int);

BOOL    WINAPI EnableWindow(HWND,BOOL);
BOOL    WINAPI IsWindowEnabled(HWND);

#if defined (TWIN32)
BOOL    WINAPI SetWindowText(HWND, LPCSTR);
#else
void    WINAPI SetWindowText(HWND, LPCSTR);
#endif
int     WINAPI GetWindowText(HWND, LPSTR, int);
int     WINAPI GetWindowTextLength(HWND);

WORD    WINAPI GetWindowWord(HWND, int);
WORD    WINAPI SetWindowWord(HWND, int, WORD);
LONG    WINAPI GetWindowLong(HWND, int);
LONG    WINAPI SetWindowLong(HWND, int, LONG);

#endif              /* NOAPIPROTO */



/* Special value for CreateWindow, et al. */
#define HWND_DESKTOP    ((HWND)0)

#define WM_CREATE       0x0001
#define WM_NCCREATE     0x0081

#define WM_DESTROY      0x0002
#define WM_NCDESTROY    0x0082

#define WM_SETREDRAW    0x000B

#define WM_ENABLE       0x000A

#define WM_SETTEXT      0x000C
#define WM_GETTEXT      0x000D
#define WM_GETTEXTLENGTH    0x000E


#ifndef NOWINSTYLES

/* Basic window types */
#define WS_OVERLAPPED       0x00000000L
#define WS_POPUP        0x80000000L
#define WS_CHILD        0x40000000L

/* Clipping styles */
#define WS_CLIPSIBLINGS     0x04000000L
#define WS_CLIPCHILDREN     0x02000000L

/* Generic window states */
#define WS_VISIBLE      0x10000000L
#define WS_DISABLED     0x08000000L

/* Main window states */
#define WS_MINIMIZE     0x20000000L
#define WS_MAXIMIZE     0x01000000L

/* Main window styles */
#define WS_CAPTION      0x00C00000L     /* WS_BORDER | WS_DLGFRAME  */
#define WS_BORDER       0x00800000L
#define WS_DLGFRAME     0x00400000L
#define WS_VSCROLL      0x00200000L
#define WS_HSCROLL      0x00100000L
#define WS_SYSMENU      0x00080000L
#define WS_THICKFRAME       0x00040000L
#define WS_MINIMIZEBOX      0x00020000L
#define WS_MAXIMIZEBOX      0x00010000L

/* Control window styles */
#define WS_GROUP        0x00020000L
#define WS_TABSTOP      0x00010000L

/* Common Window Styles */
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_POPUPWINDOW      (WS_POPUP | WS_BORDER | WS_SYSMENU)
#define WS_CHILDWINDOW      (WS_CHILD)

/* Extended Window Styles */
#define WS_EX_DLGMODALFRAME  0x00000001L
#define WS_EX_NOPARENTNOTIFY 0x00000004L
#define WS_EX_TOPMOST        0x00000008L
#define WS_EX_ACCEPTFILES    0x00000010L
#define WS_EX_TRANSPARENT    0x00000020L

#define WS_EX_MDICHILD		0x00000040L
#define WS_EX_TOOLWINDOW	0x00000080L
#define WS_EX_WINDOWEDGE	0x00000100L
#define WS_EX_CLIENTEDGE	0x00000200L
#define WS_EX_CONTEXTHELP	0x00000400L

#define WS_EX_LEFT		0x00000000L
#define WS_EX_RIGHT		0x00001000L
#define WS_EX_LTRREADING	0x00000000L
#define WS_EX_RTLREADING	0x00002000L
#define WS_EX_RIGHTSCROLLBAR	0x00000000L
#define WS_EX_LEFTSCROLLBAR	0x00004000L

#define WS_EX_CONTROLPARENT	0x00010000L
#define WS_EX_STATICEDGE	0x00020000L
#define WS_EX_APPWINDOW		0x00040000L

#define WS_EX_OVERLAPPEDWINDOW	(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
#define WS_EX_PALETTEWINDOW	\
	(WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)

/* Obsolete style names */
#define WS_TILED        WS_OVERLAPPED
#define WS_ICONIC       WS_MINIMIZE
#define WS_SIZEBOX      WS_THICKFRAME
#define WS_TILEDWINDOW      WS_OVERLAPPEDWINDOW

#endif      /* NOWINSTYLES */


#ifndef NOSHOWWINDOW

#define SW_HIDE         0
#define SW_SHOWNORMAL       1
#define SW_NORMAL       1
#define SW_SHOWMINIMIZED    2
#define SW_SHOWMAXIMIZED    3
#define SW_MAXIMIZE     3
#define SW_SHOWNOACTIVATE   4
#define SW_SHOW         5
#define SW_MINIMIZE     6
#define SW_SHOWMINNOACTIVE  7
#define SW_SHOWNA       8
#define SW_RESTORE          9

/* Obsolete ShowWindow() command names */
#define HIDE_WINDOW     0
#define SHOW_OPENWINDOW     1
#define SHOW_ICONWINDOW     2
#define SHOW_FULLSCREEN     3
#define SHOW_OPENNOACTIVATE 4

#define WM_SHOWWINDOW       0x0018

/* WM_SHOWWINDOW wParam codes */
#define SW_PARENTCLOSING    1
#define SW_OTHERMAXIMIZED   2
#define SW_PARENTOPENING    3
#define SW_OTHERRESTORED    4

/* Obsolete constant names */
#define SW_OTHERZOOM        SW_OTHERMAXIMIZED
#define SW_OTHERUNZOOM      SW_OTHERRESTORED

#endif      /* NOSHOWWINDOW */


/* Window field offsets for GetWindowLong() and GetWindowWord() */
#ifndef NOWINOFFSETS

#ifndef	TWIN32
#define GWL_WNDPROC     (-4)
#define GWW_HINSTANCE       (-6)
#define GWW_HWNDPARENT      (-8)
#define GWW_ID          (-12)
#define GWL_STYLE       (-16)
#define GWL_EXSTYLE     (-20)
#else
#define GWL_WNDPROC     (-4)
#define GWL_HINSTANCE       (-6)
#define GWL_HWNDPARENT      (-8)
#define GWL_ID          (-12)
#define GWL_STYLE       (-16)
#define GWL_EXSTYLE     (-20)
#endif

/* The following are the WIN32 extensions supported in WIN16 */
#define GWL_USERDATA    (-21)

#endif  /* NOWINOFFSETS */



/* Window size management ******************************************/

typedef struct tagWINDOWPLACEMENT
{
    UINT  length;
    UINT  flags;
    UINT  showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT  rcNormalPosition;
} WINDOWPLACEMENT;
typedef WINDOWPLACEMENT     *PWINDOWPLACEMENT;
typedef WINDOWPLACEMENT FAR* LPWINDOWPLACEMENT;

typedef struct tagSTYLESTRUCT
{
    DWORD styleOld;
    DWORD styleNew;
} STYLESTRUCT;
typedef STYLESTRUCT		*PSTYLESTRUCT;
typedef STYLESTRUCT NEAR	*NPSTYLESTRUCT;
typedef STYLESTRUCT FAR		*LPSTYLESTRUCT;

/* WM_WINDOWPOSCHANGING/CHANGED struct pointed to by lParam */
typedef struct tagWINDOWPOS
{
    HWND    hwnd;
    HWND    hwndInsertAfter;
    int     x;
    int     y;
    int     cx;
    int     cy;
    UINT    flags;
} WINDOWPOS;
typedef WINDOWPOS FAR* LPWINDOWPOS;


#ifndef NOAPIPROTO  /* Prototypes */

BOOL    WINAPI GetClientRect(HWND, RECT FAR*);
BOOL    WINAPI GetWindowRect(HWND, RECT FAR*);

BOOL    WINAPI GetWindowPlacement(HWND, WINDOWPLACEMENT FAR*);
BOOL    WINAPI SetWindowPlacement(HWND, const WINDOWPLACEMENT FAR*);

BOOL    WINAPI SetWindowPos(HWND, HWND, int, int, int, int, UINT);

BOOL    WINAPI MoveWindow(HWND, int, int, int, int, BOOL);
BOOL    WINAPI BringWindowToTop(HWND);

#endif              /* NOAPIPROTO */


#define CW_USEDEFAULT   ((UINT)0x80000000)      /* ((int)0x8000) (orig) */

#define WPF_SETMINPOSITION      0x0001
#define WPF_RESTORETOMAXIMIZED  0x0002

/* SetWindowPos() and WINDOWPOS flags */
#define SWP_NOSIZE          0x0001
#define SWP_NOMOVE          0x0002
#define SWP_NOZORDER        0x0004
#define SWP_NOREDRAW        0x0008
#define SWP_NOACTIVATE      0x0010
#define SWP_FRAMECHANGED    0x0020  /* The frame changed: send WM_NCCALCSIZE */
#define SWP_SHOWWINDOW      0x0040
#define SWP_HIDEWINDOW      0x0080
#define SWP_NOCOPYBITS      0x0100
#define SWP_NOOWNERZORDER   0x0200  /* Don't do owner Z ordering */

#define SWP_DRAWFRAME       SWP_FRAMECHANGED
#define SWP_NOREPOSITION    SWP_NOOWNERZORDER

#define SWP_NOSENDCHANGING  0x0400
#define SWP_DEFERERASE      0x2000

/* SetWindowPos() hwndInsertAfter field values */
#define HWND_TOP            ((HWND)0)
#define HWND_BOTTOM     ((HWND)1)
#define HWND_TOPMOST        ((HWND)-1)
#define HWND_NOTOPMOST      ((HWND)-2)

#define WM_WINDOWPOSCHANGING 0x0046
#define WM_WINDOWPOSCHANGED 0x0047

#define WM_NOTIFY                       0x004E
#define WM_INPUTLANGCHANGEREQUEST       0x0050
#define WM_INPUTLANGCHANGE              0x0051
#define WM_TCARD                        0x0052
#define WM_HELP                         0x0053
#define WM_USERCHANGED                  0x0054
#define WM_NOTIFYFORMAT                 0x0055

#define NFR_ANSI                             1
#define NFR_UNICODE                          2
#define NF_QUERY                             3
#define NF_REQUERY                           4

#define WM_CONTEXTMENU                  0x007B
#define WM_STYLECHANGING                0x007C
#define WM_STYLECHANGED                 0x007D
#define WM_DISPLAYCHANGE                0x007E
#define WM_GETICON                      0x007F
#define WM_SETICON                      0x0080

typedef struct tagNMHDR
{
    HWND  hwndFrom;
    UINT  idFrom;
    UINT  code;
}   NMHDR;
typedef NMHDR FAR * LPNMHDR;

typedef struct tagDLGTEMPLATE
{
    DWORD style;
    DWORD dwExtendedStyle;
    WORD  cdit;
    short x;
    short y;
    short cx;
    short cy;
} DLGTEMPLATE, *LPDLGTEMPLATE;

typedef CONST DLGTEMPLATE *LPCDLGTEMPLATE;

typedef struct tabDLGITEMTEMPLATE{
    DWORD style;
    DWORD dwExtendedStyle;
    short x;
    short y;
    short cx;
    short cy;
    WORD id;
} DLGITEMTEMPLATE, *LPDLGITEMTEMPLATE;

typedef struct _ICONINFO {

   BOOL fIcon;
   DWORD xHotspot;
   DWORD yHotspot;
   HBITMAP hbmMask;
   HBITMAP hbmColor;
} ICONINFO, *PICONINFO, *LPICONINFO;

#define IMAGE_BITMAP   0
#define IMAGE_ICON     1
#define IMAGE_CURSOR   2
#define IMAGE_ENHMETAFILE  3

#define LR_DEFAULTCOLOR	  0x0000
#define LR_MONOCHROME	  0x0001
#define LR_COLOR	  0x0002
#define LR_LOADFROMFILE   0x0010

#define WM_MOVE         0x0003
#define WM_SIZE         0x0005

/* WM_SIZE message wParam values */
#define SIZE_RESTORED       0
#define SIZE_MINIMIZED      1
#define SIZE_MAXIMIZED      2
#define SIZE_MAXSHOW        3
#define SIZE_MAXHIDE        4

/* Obsolete constant names */
#define SIZENORMAL      SIZE_RESTORED
#define SIZEICONIC      SIZE_MINIMIZED
#define SIZEFULLSCREEN      SIZE_MAXIMIZED
#define SIZEZOOMSHOW        SIZE_MAXSHOW
#define SIZEZOOMHIDE        SIZE_MAXHIDE


#ifndef NODEFERWINDOWPOS

DECLARE_HANDLE(HDWP);

#ifndef NOAPIPROTO  /* Prototypes */

HDWP    WINAPI BeginDeferWindowPos(int);
HDWP    WINAPI DeferWindowPos(HDWP, HWND, HWND, int, int, int, int, UINT);
BOOL    WINAPI EndDeferWindowPos(HDWP);

#endif              /* NOAPIPROTO */

#endif  /* NODEFERWINDOWPOS */



/* Window proc support *********************************************/

/* WH_CALLWNDPROC Message structure */

typedef struct tagCWPSTRUCT {
    LPARAM  lParam;
    WPARAM  wParam;
    UINT    message;
    HWND    hwnd;
} CWPSTRUCT;

typedef CWPSTRUCT      *PCWPSTRUCT;
typedef CWPSTRUCT NEAR *NPCWPSTRUCT;
typedef CWPSTRUCT FAR  *LPCWPSTRUCT;

#ifndef NOAPIPROTO  /* Prototypes */

LRESULT WINAPI DefWindowProc(HWND, UINT, WPARAM, LPARAM);

#ifdef STRICT
LRESULT WINAPI CallWindowProc(WNDPROC, HWND, UINT, WPARAM, LPARAM);
#else
LRESULT WINAPI CallWindowProc(FARPROC, HWND, UINT, WPARAM, LPARAM);
#endif

#endif              /* NOAPIPROTO */



/* Main window support *********************************************/

/* Struct pointed to by WM_GETMINMAXINFO lParam */
typedef struct tagMINMAXINFO
{
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
} MINMAXINFO;

typedef MINMAXINFO *PMINMAXINFO;
typedef MINMAXINFO *LPMINMAXINFO;

#ifndef NOAPIPROTO  /* Prototypes */

void    WINAPI AdjustWindowRect(RECT FAR*, DWORD, BOOL);
void    WINAPI AdjustWindowRectEx(RECT FAR*, DWORD, BOOL, DWORD);

BOOL    WINAPI FlashWindow(HWND, BOOL);

void    WINAPI ShowOwnedPopups(HWND, BOOL);

/* Obsolete functions */
BOOL    WINAPI OpenIcon(HWND);
void    WINAPI CloseWindow(HWND);
BOOL    WINAPI AnyPopup(void);
BOOL    WINAPI IsIconic(HWND);
BOOL    WINAPI IsZoomed(HWND);

#endif              /* NOAPIPROTO */


#define WM_QUERYOPEN        0x0013
#define WM_CLOSE        0x0010
#define WM_GETMINMAXINFO    0x0024



/* Window coordinate mapping and hit testing ***********************/

#ifndef NOAPIPROTO  /* Prototypes */

BOOL    WINAPI ClientToScreen(HWND, POINT FAR*);
BOOL    WINAPI ScreenToClient(HWND, POINT FAR*);

void    WINAPI MapWindowPoints(HWND hwndFrom, HWND hwndTo, POINT FAR* lppt, UINT cpt);

HWND    WINAPI WindowFromPoint(POINT);
HWND    WINAPI ChildWindowFromPoint(HWND, POINT);

#endif              /* NOAPIPROTO */



/* Window query and enumeration ************************************/

#ifdef STRICT
typedef BOOL (CALLBACK* WNDENUMPROC)(HWND, LPARAM);
#else
typedef FARPROC WNDENUMPROC;
#endif


#ifndef NOAPIPROTO  /* Prototypes */

HWND    WINAPI GetDesktopWindow(void);

HWND    WINAPI FindWindow(LPCSTR, LPCSTR);

BOOL    WINAPI EnumWindows(WNDENUMPROC, LPARAM);
BOOL    WINAPI EnumChildWindows(HWND, WNDENUMPROC, LPARAM);
BOOL    WINAPI EnumTaskWindows(HTASK, WNDENUMPROC, LPARAM);

HWND    WINAPI GetTopWindow(HWND);

HWND    WINAPI GetWindow(HWND, UINT);
HWND    WINAPI GetNextWindow(HWND, UINT);

#endif              /* NOAPIPROTO */


/* GetWindow() constants */
#define GW_HWNDFIRST    0
#define GW_HWNDLAST 1
#define GW_HWNDNEXT 2
#define GW_HWNDPREV 3
#define GW_OWNER    4
#define GW_CHILD    5



/* Window property support *****************************************/

#ifdef STRICT
typedef BOOL (CALLBACK* PROPENUMPROC)(HWND, LPCSTR, HANDLE);
#else
typedef FARPROC PROPENUMPROC;
#endif


#ifndef NOAPIPROTO  /* Prototypes */

BOOL    WINAPI SetProp(HWND, LPCSTR, HANDLE);
HANDLE  WINAPI GetProp(HWND, LPCSTR);
HANDLE  WINAPI RemoveProp(HWND, LPCSTR);

int     WINAPI EnumProps(HWND, PROPENUMPROC);

#endif              /* NOAPIPROTO */


/* Window drawing support ******************************************/

#ifndef NOAPIPROTO  /* Prototypes */

HDC     WINAPI GetDC(HWND);
HDC     WINAPI GetDCEx(register HWND hwnd, HRGN hrgnClip, DWORD flags);
HDC     WINAPI GetWindowDC(HWND);
int     WINAPI ReleaseDC(HWND, HDC);
HWND	WINAPI WindowFromDC(HDC);

#endif	/* NOAPIPROTO */

#define DCX_WINDOW		0x00000001L
#define DCX_CACHE		0x00000002L
#define DCX_CLIPCHILDREN	0x00000008L
#define DCX_CLIPSIBLINGS	0x00000010L
#define DCX_PARENTCLIP		0x00000020L
#define DCX_EXCLUDERGN		0x00000040L
#define DCX_INTERSECTRGN	0x00000080L
#define DCX_LOCKWINDOWUPDATE	0x00000400L
#define DCX_USESTYLE		0x00010000L

#define FVIRTKEY		TRUE
#define FNOINVERT		0x02

/* avoid redefinition with solaris's sys/param.h caused by TWIN's winsock.h */

#undef  FSHIFT
#define FSHIFT    0x04
#define FCONTROL  0x08
#define FALT      0x10

typedef struct tagACCEL {
	BYTE	fVirt;
	WORD	key;
	WORD	cmd;
} ACCEL, *LPACCEL;


/* Window repainting support ***************************************/

/* BeginPaint() return structure */
typedef struct tagPAINTSTRUCT
{
    HDC     hdc;
    BOOL    fErase;
    RECT    rcPaint;
    BOOL    fRestore;
    BOOL    fIncUpdate;
    BYTE    rgbReserved[16];
} PAINTSTRUCT;
typedef PAINTSTRUCT* PPAINTSTRUCT;
typedef PAINTSTRUCT NEAR* NPPAINTSTRUCT;
typedef PAINTSTRUCT FAR* LPPAINTSTRUCT;


#ifndef NOAPIPROTO  /* Prototypes */

HDC     WINAPI BeginPaint(HWND, PAINTSTRUCT FAR*);
void    WINAPI EndPaint(HWND, const PAINTSTRUCT FAR*);

void    WINAPI UpdateWindow(HWND);

int     WINAPI ExcludeUpdateRgn(HDC, HWND);

BOOL    WINAPI LockWindowUpdate(HWND hwndLock);

BOOL    WINAPI GetUpdateRect(HWND, RECT FAR*, BOOL);
int     WINAPI GetUpdateRgn(HWND, HRGN, BOOL);

void    WINAPI InvalidateRect(HWND, const RECT FAR*, BOOL);
void    WINAPI ValidateRect(HWND, const RECT FAR*);

void    WINAPI InvalidateRgn(HWND, HRGN, BOOL);
void    WINAPI ValidateRgn(HWND, HRGN);

BOOL    WINAPI RedrawWindow(HWND hwnd, const RECT FAR* lprcUpdate, HRGN hrgnUpdate, UINT flags);

#endif              /* NOAPIPROTO */


#define WM_PAINT        0x000F
#define WM_ERASEBKGND       0x0014
#define WM_ICONERASEBKGND   0x0027

#define RDW_INVALIDATE      0x0001
#define RDW_INTERNALPAINT   0x0002
#define RDW_ERASE       0x0004

#define RDW_VALIDATE        0x0008
#define RDW_NOINTERNALPAINT 0x0010
#define RDW_NOERASE     0x0020

#define RDW_NOCHILDREN      0x0040
#define RDW_ALLCHILDREN     0x0080

#define RDW_UPDATENOW       0x0100
#define RDW_ERASENOW        0x0200

#define RDW_FRAME               0x0400
#define RDW_NOFRAME             0x0800



/* Window scrolling support ****************************************/

#ifndef NOAPIPROTO  /* Prototypes */

void    WINAPI ScrollWindow(HWND, int, int, const RECT FAR*, const RECT FAR*);
BOOL    WINAPI ScrollDC(HDC, int, int, const RECT FAR*, const RECT FAR*, HRGN, RECT FAR*);

int     WINAPI ScrollWindowEx(HWND hwnd, int dx, int dy,
                const RECT FAR* prcScroll, const RECT FAR* prcClip,
                HRGN hrgnUpdate, RECT FAR* prcUpdate, UINT flags);

#endif              /* NOAPIPROTO */


#define SW_SCROLLCHILDREN   0x0001
#define SW_INVALIDATE       0x0002
#define SW_ERASE            0x0004



/* Non client area support *****************************************/

/* WM_NCCALCSIZE parameter structure */
typedef struct tagNCCALCSIZE_PARAMS
{
    RECT       rgrc[3];
    WINDOWPOS FAR* lppos;
} NCCALCSIZE_PARAMS;
typedef NCCALCSIZE_PARAMS FAR* LPNCCALCSIZE_PARAMS;


#define WM_NCPAINT      0x0085

#define WM_NCCALCSIZE       0x0083

#define WVR_ALIGNTOP        0x0010
#define WVR_ALIGNLEFT       0x0020
#define WVR_ALIGNBOTTOM     0x0040
#define WVR_ALIGNRIGHT      0x0080
#define WVR_HREDRAW     0x0100
#define WVR_VREDRAW     0x0200
#define WVR_REDRAW      (WVR_HREDRAW | WVR_VREDRAW)
#define WVR_VALIDRECTS      0x0400

#define WM_NCHITTEST        0x0084

/* WM_NCHITTEST return codes */
#define HTERROR		(-2)
#define HTTRANSPARENT	(-1)
#define HTNOWHERE	0
#define HTCLIENT        1
#define HTCAPTION       2
#define HTSYSMENU       3
#define HTSIZE          4
#define HTMENU          5
#define HTHSCROLL       6
#define HTVSCROLL       7
#define HTMINBUTTON     8
#define HTMAXBUTTON     9
#define HTLEFT          10
#define HTRIGHT         11
#define HTTOP           12
#define HTTOPLEFT       13
#define HTTOPRIGHT      14
#define HTBOTTOM        15
#define HTBOTTOMLEFT	16
#define HTBOTTOMRIGHT	17
#define HTBORDER	18
#define HTGROWBOX       HTSIZE
#define HTREDUCE        HTMINBUTTON
#define HTZOOM          HTMAXBUTTON
#define HTSIZEFIRST	HTLEFT
#define HTSIZELAST	HTBOTTOMRIGHT

#if (WINVER >= 0x0400)
#define HTOBJECT	19
#define HTCLOSE		20
#define HTHELP		21
#endif	/* WINVER */

/* Drag and drop support *******************************************/

#define WM_QUERYDRAGICON    0x0037
#define WM_DROPFILES        0x0233


/* Window activation support ***************************************/

#ifndef NOAPIPROTO  /* Prototypes */

HWND    WINAPI SetActiveWindow(HWND);
HWND    WINAPI GetActiveWindow(void);

HWND    WINAPI GetLastActivePopup(HWND);

#endif              /* NOAPIPROTO */


/* WM_ACTIVATE state values */
#define WA_INACTIVE     0
#define WA_ACTIVE       1
#define WA_CLICKACTIVE      2

#define WM_ACTIVATE     0x0006
#define WM_ACTIVATEAPP      0x001C
#define WM_NCACTIVATE       0x0086


/* Keyboard support ************************************************/

#ifndef NOAPIPROTO  /* Prototypes */

HWND    WINAPI SetFocus(HWND);
HWND    WINAPI GetFocus(void);

int     WINAPI GetKeyState(int);
int     WINAPI GetAsyncKeyState(int);

void    WINAPI GetKeyboardState(BYTE FAR* );
void    WINAPI SetKeyboardState(BYTE FAR* );


#endif              /* NOAPIPROTO */


#define WM_SETFOCUS     0x0007
#define WM_KILLFOCUS        0x0008

#define WM_KEYDOWN      0x0100
#define WM_KEYUP        0x0101

#define WM_CHAR         0x0102
#define WM_DEADCHAR     0x0103

#define WM_SYSKEYDOWN       0x0104
#define WM_SYSKEYUP     0x0105

#define WM_SYSCHAR      0x0106
#define WM_SYSDEADCHAR      0x0107


/* Keyboard message range */
#define WM_KEYFIRST     0x0100
#define WM_KEYLAST      0x0108

/* WM_KEYUP/DOWN/CHAR HIWORD(lParam) flags */
#define KF_EXTENDED     0x0100
#define KF_DLGMODE      0x0800
#define KF_MENUMODE     0x1000
#define KF_ALTDOWN      0x2000
#define KF_REPEAT       0x4000
#define KF_UP           0x8000

/* Virtual key codes */
#ifndef NOVIRTUALKEYCODES

#define VK_LBUTTON      0x01
#define VK_RBUTTON      0x02
#define VK_CANCEL       0x03
#define VK_MBUTTON          0x04
#define VK_BACK         0x08
#define VK_TAB          0x09
#define VK_CLEAR        0x0C
#define VK_RETURN       0x0D
#define VK_SHIFT        0x10
#define VK_CONTROL      0x11
#define VK_MENU         0x12
#define VK_PAUSE        0x13
#define VK_CAPITAL      0x14
#define VK_ESCAPE       0x1B
#define VK_SPACE        0x20
#define VK_PRIOR        0x21
#define VK_NEXT         0x22
#define VK_END          0x23
#define VK_HOME         0x24
#define VK_LEFT         0x25
#define VK_UP           0x26
#define VK_RIGHT        0x27
#define VK_DOWN         0x28
#define VK_SELECT       0x29
#define VK_PRINT        0x2A
#define VK_EXECUTE      0x2B
#define VK_SNAPSHOT     0x2C
#define VK_INSERT       0x2D
#define VK_DELETE       0x2E
#define VK_HELP         0x2F
#define VK_NUMPAD0      0x60
#define VK_NUMPAD1      0x61
#define VK_NUMPAD2      0x62
#define VK_NUMPAD3      0x63
#define VK_NUMPAD4      0x64
#define VK_NUMPAD5      0x65
#define VK_NUMPAD6      0x66
#define VK_NUMPAD7      0x67
#define VK_NUMPAD8      0x68
#define VK_NUMPAD9      0x69
#define VK_MULTIPLY     0x6A
#define VK_ADD          0x6B
#define VK_SEPARATOR        0x6C
#define VK_SUBTRACT     0x6D
#define VK_DECIMAL      0x6E
#define VK_DIVIDE       0x6F
#define VK_F1           0x70
#define VK_F2           0x71
#define VK_F3           0x72
#define VK_F4           0x73
#define VK_F5           0x74
#define VK_F6           0x75
#define VK_F7           0x76
#define VK_F8           0x77
#define VK_F9           0x78
#define VK_F10          0x79
#define VK_F11          0x7A
#define VK_F12          0x7B
#define VK_F13          0x7C
#define VK_F14          0x7D
#define VK_F15          0x7E
#define VK_F16          0x7F
#define VK_F17          0x80
#define VK_F18          0x81
#define VK_F19          0x82
#define VK_F20          0x83
#define VK_F21          0x84
#define VK_F22          0x85
#define VK_F23          0x86
#define VK_F24          0x87
#define VK_NUMLOCK      0x90
#define VK_SCROLL           0x91

/* VK_A thru VK_Z are the same as their ASCII equivalents: 'A' thru 'Z' */
/* VK_0 thru VK_9 are the same as their ASCII equivalents: '0' thru '0' */

#endif  /* NOVIRTUALKEYCODES */



/* Mouse support ***************************************************/

#ifndef NOAPIPROTO  /* Prototypes */

HWND    WINAPI SetCapture(HWND);
HWND    WINAPI GetCapture(void);
#ifdef	TWIN32
BOOL    WINAPI ReleaseCapture(void);
#else
void    WINAPI ReleaseCapture(void);
#endif

BOOL    WINAPI SwapMouseButton(BOOL);


#endif              /* NOAPIPROTO */


/* Mouse input messages */
#define WM_MOUSEMOVE        0x0200
#define WM_LBUTTONDOWN      0x0201
#define WM_LBUTTONUP        0x0202
#define WM_LBUTTONDBLCLK    0x0203
#define WM_RBUTTONDOWN      0x0204
#define WM_RBUTTONUP        0x0205
#define WM_RBUTTONDBLCLK    0x0206
#define WM_MBUTTONDOWN      0x0207
#define WM_MBUTTONUP        0x0208
#define WM_MBUTTONDBLCLK    0x0209

/* Mouse input message range */
#define WM_MOUSEFIRST       0x0200
#define WM_MOUSELAST        0x0209

/* Mouse message wParam key states */
#ifndef NOKEYSTATES

#define MK_LBUTTON      0x0001
#define MK_RBUTTON      0x0002
#define MK_SHIFT        0x0004
#define MK_CONTROL      0x0008
#define MK_MBUTTON      0x0010

#endif  /* NOKEYSTATES */

/* Non-client mouse messages */
#define WM_NCMOUSEMOVE      0x00A0
#define WM_NCLBUTTONDOWN    0x00A1
#define WM_NCLBUTTONUP      0x00A2
#define WM_NCLBUTTONDBLCLK  0x00A3
#define WM_NCRBUTTONDOWN    0x00A4
#define WM_NCRBUTTONUP      0x00A5
#define WM_NCRBUTTONDBLCLK  0x00A6
#define WM_NCMBUTTONDOWN    0x00A7
#define WM_NCMBUTTONUP      0x00A8
#define WM_NCMBUTTONDBLCLK  0x00A9

/* Mouse click activation support */
#define WM_MOUSEACTIVATE    0x0021

/* WM_MOUSEACTIVATE return codes */
#define MA_ACTIVATE             1
#define MA_ACTIVATEANDEAT       2
#define MA_NOACTIVATE           3
#define MA_NOACTIVATEANDEAT     4

/* SetWindowsHook() keyboard hook */
#define WH_KEYBOARD     2


/* SetWindowsHook() mouse hook */
#ifndef NOWH

#define WH_MOUSE        7

typedef struct tagMOUSEHOOKSTRUCT
{
    POINT   pt;
    HWND    hwnd;
    UINT    wHitTestCode;
    DWORD   dwExtraInfo;
} MOUSEHOOKSTRUCT;
typedef MOUSEHOOKSTRUCT  FAR* LPMOUSEHOOKSTRUCT;

#endif  /* NOWH */



/* Mode control ****************************************************/

#define WM_CANCELMODE       0x001F



/* System modal window support *************************************/

#ifndef NOAPIPROTO  /* Prototypes */

HWND    WINAPI GetSysModalWindow(void);
HWND    WINAPI SetSysModalWindow(HWND);

#endif              /* NOAPIPROTO */



/* Timer support ***************************************************/

#ifdef STRICT
typedef void (CALLBACK* TIMERPROC)(HWND, UINT, UINT, DWORD);
#else
typedef FARPROC TIMERPROC;
#endif


#ifndef NOAPIPROTO  /* Prototypes */

UINT    WINAPI SetTimer(HWND, UINT, UINT, TIMERPROC);

BOOL    WINAPI KillTimer(HWND, UINT);

#endif              /* NOAPIPROTO */


#define WM_TIMER        0x0113



/* Accelerator support *********************************************/

DECLARE_HANDLE(HACCEL);

#ifndef NOAPIPROTO  /* Prototypes */

HACCEL  WINAPI LoadAccelerators(HINSTANCE, LPCSTR);

int	WINAPI CopyAcceleratorTable(HACCEL hAccelTable, LPACCEL lpAccelTable,
	int nAccelTableSize);

#ifndef NOMSG
int     WINAPI TranslateAccelerator(HWND, HACCEL, MSG FAR*);
#endif

#endif              /* NOAPIPROTO */



/* Menu support ****************************************************/

/* Menu messages */
#define WM_INITMENU         0x0116
#define WM_INITMENUPOPUP    0x0117


/* Menu and control command messages */
#define WM_COMMAND          0x0111


#ifndef NOMENUS

/* Menu template header */
typedef struct
{
    UINT    versionNumber;
    UINT    offset;
} MENUITEMTEMPLATEHEADER;

/* Menu template item struct */
typedef struct
{
    UINT    mtOption;
    UINT    mtID;
    char    mtString[1];
} MENUITEMTEMPLATE;


#ifndef NOAPIPROTO  /* Prototypes */

BOOL    WINAPI IsMenu(HMENU);

HMENU   WINAPI CreateMenu(void);
HMENU   WINAPI CreatePopupMenu(void);
HMENU   WINAPI LoadMenu(HINSTANCE, LPCSTR);
HMENU   WINAPI LoadMenuIndirect(const void *);

BOOL    WINAPI DestroyMenu(HMENU);

HMENU   WINAPI GetMenu(HWND);
BOOL    WINAPI SetMenu(HWND, HMENU);

HMENU   WINAPI GetSystemMenu(HWND, BOOL);

void    WINAPI DrawMenuBar(HWND);

BOOL    WINAPI HiliteMenuItem(HWND, HMENU, UINT, UINT);

BOOL    WINAPI InsertMenu(HMENU, UINT, UINT, UINT, LPCSTR);
BOOL    WINAPI AppendMenu(HMENU, UINT, UINT, LPCSTR);
BOOL    WINAPI ModifyMenu(HMENU, UINT, UINT, UINT, LPCSTR);
BOOL    WINAPI RemoveMenu(HMENU, UINT, UINT);
BOOL    WINAPI DeleteMenu(HMENU, UINT, UINT);

BOOL    WINAPI ChangeMenu(HMENU, UINT, LPCSTR, UINT, UINT);

BOOL    WINAPI EnableMenuItem(HMENU, UINT, UINT);
BOOL    WINAPI CheckMenuItem(HMENU, UINT, UINT);

HMENU   WINAPI GetSubMenu(HMENU, int);

int     WINAPI GetMenuItemCount(HMENU);
UINT    WINAPI GetMenuItemID(HMENU, int);

int     WINAPI GetMenuString(HMENU, UINT, LPSTR, int, UINT);
UINT    WINAPI GetMenuState(HMENU, UINT, UINT);

BOOL    WINAPI SetMenuItemBitmaps(HMENU, UINT, UINT, HBITMAP, HBITMAP);
DWORD   WINAPI GetMenuCheckMarkDimensions(void);

BOOL    WINAPI TrackPopupMenu(HMENU, UINT, int, int, int, HWND, const RECT FAR*);

#endif              /* NOAPIPROTO */


#define WM_MENUSELECT       0x011F
#define WM_MENUCHAR     0x0120

#define MF_INSERT       0x0000
#define MF_CHANGE       0x0080
#define MF_APPEND       0x0100
#define MF_DELETE       0x0200
#define MF_REMOVE       0x1000

/* Menu flags for Add/Check/EnableMenuItem() */
#define MF_BYCOMMAND    0x0000
#define MF_BYPOSITION   0x0400

#define MF_SEPARATOR    0x0800

#define MF_ENABLED      0x0000
#define MF_GRAYED       0x0001
#define MF_DISABLED     0x0002

#define MF_UNCHECKED    0x0000
#define MF_CHECKED      0x0008
#define MF_USECHECKBITMAPS  0x0200

#define MF_STRING       0x0000
#define MF_BITMAP       0x0004
#define MF_OWNERDRAW    0x0100

#define MF_POPUP        0x0010
#define MF_MENUBARBREAK 0x0020
#define MF_MENUBREAK    0x0040

#define MF_UNHILITE     0x0000
#define MF_HILITE       0x0080

#define MF_SYSMENU      0x2000
#define MF_HELP         0x4000
#define MF_MOUSESELECT  0x8000

#define MF_END          0x0080  /* Only valid in menu resource templates */

/* Flags for TrackPopupMenu */
#define TPM_LEFTBUTTON  0x0000
#define TPM_RIGHTBUTTON 0x0002
#define TPM_LEFTALIGN   0x0000
#define TPM_CENTERALIGN 0x0004
#define TPM_RIGHTALIGN  0x0008


#endif      /* NOMENUS */



/* Scroll bar support **********************************************/

#ifndef NOSCROLL

typedef struct tagSCROLLINFO {
	UINT	cbSize;
	UINT	fMask;
	int	nMin;
	int	nMax;
	UINT	nPage;
	int	nPos;
	int	nTrackPos;
} SCROLLINFO;
typedef SCROLLINFO	*PSCROLLINFO;
typedef SCROLLINFO NEAR	*NPSCROLLINFO;
typedef SCROLLINFO FAR	*LPSCROLLINFO;

#ifndef NOAPIPROTO  /* Prototypes */

int     WINAPI SetScrollPos(HWND, int, int, BOOL);
int     WINAPI GetScrollPos(HWND, int);
void    WINAPI SetScrollRange(HWND, int, int, int, BOOL);
void    WINAPI GetScrollRange(HWND, int, int FAR*, int FAR*);
void    WINAPI ShowScrollBar(HWND, int, BOOL);
BOOL    WINAPI EnableScrollBar(HWND, int, UINT);
BOOL	WINAPI GetScrollInfo(HWND, int, LPSCROLLINFO);
int	WINAPI SetScrollInfo(HWND, int, LPSCROLLINFO, BOOL);

#endif              /* NOAPIPROTO */

#define WM_HSCROLL      0x0114
#define WM_VSCROLL      0x0115

/* WM_H/VSCROLL commands */
#define SB_LINEUP       0
#define SB_LINELEFT     0
#define SB_LINEDOWN     1
#define SB_LINERIGHT        1
#define SB_PAGEUP       2
#define SB_PAGELEFT     2
#define SB_PAGEDOWN     3
#define SB_PAGERIGHT        3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP          6
#define SB_LEFT         6
#define SB_BOTTOM       7
#define SB_RIGHT        7
#define SB_ENDSCROLL        8

/* Scroll bar selection constants */
#define SB_HORZ         0
#define SB_VERT         1
#define SB_CTL          2
#define SB_BOTH         3

/* EnableScrollBar() flags */
#define ESB_ENABLE_BOTH     0x0000
#define ESB_DISABLE_BOTH    0x0003

#define ESB_DISABLE_LEFT    0x0001
#define ESB_DISABLE_RIGHT   0x0002

#define ESB_DISABLE_UP      0x0001
#define ESB_DISABLE_DOWN    0x0002

#define ESB_DISABLE_LTUP    ESB_DISABLE_LEFT
#define ESB_DISABLE_RTDN    ESB_DISABLE_RIGHT

/* scroll bar messages */

#ifndef NOWINMESSAGES
#define SBM_SETPOS		0x00E0
#define SBM_GETPOS		0x00E1
#define SBM_SETRANGE		0x00E2
#define SBM_SETRANGEREDRAW	0x00E6
#define SBM_GETRANGE		0x00E3
#define SBM_ENABLE_ARROWS	0x00E4
#define SBM_SETSCROLLINFO	0x00E9
#define SBM_GETSCROLLINFO	0x00EA
#endif

/* GetScrollInfo() / SetScrollInfo() flags */

#define SIF_RANGE		0x0001
#define SIF_PAGE		0x0002
#define SIF_POS			0x0004
#define SIF_DISABLENOSCROLL	0x0008
#define SIF_TRACKPOS		0x0010
#define SIF_ALL			(SIF_RANGE|SIF_PAGE|SIF_POS|SIF_TRACKPOS)

#endif      /* NOSCROLL */

/* Clipboard management ********************************************/
#ifndef NOCLIPBOARD

#ifndef NOAPIPROTO  /* Prototypes */

/* Clipboard Manager Functions */
BOOL    WINAPI OpenClipboard(HWND);
BOOL    WINAPI CloseClipboard(void);
BOOL    WINAPI EmptyClipboard(void);

HWND    WINAPI GetOpenClipboardWindow(void);

HWND    WINAPI GetClipboardOwner(void);

HWND    WINAPI SetClipboardViewer(HWND);
HWND    WINAPI GetClipboardViewer(void);

HANDLE  WINAPI SetClipboardData(UINT, HANDLE);
HANDLE  WINAPI GetClipboardData(UINT);

BOOL    WINAPI IsClipboardFormatAvailable(UINT);
int     WINAPI GetPriorityClipboardFormat(UINT FAR*, int);

UINT    WINAPI RegisterClipboardFormat(LPCSTR);
int     WINAPI CountClipboardFormats(void);
UINT    WINAPI EnumClipboardFormats(UINT);
int     WINAPI GetClipboardFormatName(UINT, LPSTR, int);

BOOL    WINAPI ChangeClipboardChain(HWND, HWND);

#endif              /* NOAPIPROTO */



/* Predefined Clipboard Formats */
#define CF_TEXT          1
#define CF_BITMAP        2
#define CF_METAFILEPICT      3
#define CF_SYLK          4
#define CF_DIF           5
#define CF_TIFF          6
#define CF_OEMTEXT       7
#define CF_DIB           8
#define CF_PALETTE       9
#define CF_PENDATA          10
#define CF_RIFF             11
#define CF_WAVE             12

#define CF_OWNERDISPLAY     0x0080
#define CF_DSPTEXT      0x0081
#define CF_DSPBITMAP        0x0082
#define CF_DSPMETAFILEPICT  0x0083

/* "Private" formats don't get GlobalFree()'d */
#define CF_PRIVATEFIRST     0x0200
#define CF_PRIVATELAST      0x02FF

/* "GDIOBJ" formats do get DeleteObject()'d */
#define CF_GDIOBJFIRST      0x0300
#define CF_GDIOBJLAST       0x03FF

/* Clipboard command messages */
#define WM_CUT          0x0300
#define WM_COPY         0x0301
#define WM_PASTE        0x0302
#define WM_CLEAR        0x0303
#define WM_UNDO         0x0304

/* Clipboard owner messages */
#define WM_RENDERFORMAT     0x0305
#define WM_RENDERALLFORMATS 0x0306
#define WM_DESTROYCLIPBOARD 0x0307

/* Clipboard viewer messages */
#define WM_DRAWCLIPBOARD    0x0308
#define WM_PAINTCLIPBOARD   0x0309
#define WM_SIZECLIPBOARD    0x030B
#define WM_VSCROLLCLIPBOARD 0x030A
#define WM_HSCROLLCLIPBOARD 0x030E
#define WM_ASKCBFORMATNAME  0x030C
#define WM_CHANGECBCHAIN    0x030D



#endif      /* NOCLIPBOARD */



/* Mouse cursor management *****************************************/

#ifndef NOAPIPROTO  /* Prototypes */

HCURSOR WINAPI LoadCursor(HINSTANCE, LPCSTR);
HCURSOR WINAPI CreateCursor(HINSTANCE, int, int, int, int, const void FAR*, const void FAR*);
BOOL    WINAPI DestroyCursor(HCURSOR);

#if !defined(TWIN32)
HCURSOR WINAPI CopyCursor(HINSTANCE, HCURSOR);
#else
HCURSOR WINAPI CopyCursor(HCURSOR);
#endif

int     WINAPI ShowCursor(BOOL);

#if defined (TWIN32)
BOOL    WINAPI SetCursorPos(int, int);
BOOL    WINAPI GetCursorPos(POINT FAR*);
#else
void    WINAPI SetCursorPos(int, int);
void    WINAPI GetCursorPos(POINT FAR*);
#endif

HCURSOR WINAPI SetCursor(HCURSOR);

HCURSOR WINAPI GetCursor(void);

void    WINAPI ClipCursor(const RECT FAR*);

void    WINAPI GetClipCursor(RECT FAR*);

#endif              /* NOAPIPROTO */


/* Standard cursor resource IDs */
#define IDC_ARROW       MAKEINTRESOURCE(32512)
#define IDC_IBEAM       MAKEINTRESOURCE(32513)
#define IDC_WAIT        MAKEINTRESOURCE(32514)
#define IDC_CROSS       MAKEINTRESOURCE(32515)
#define IDC_UPARROW     MAKEINTRESOURCE(32516)
#define IDC_SIZE        MAKEINTRESOURCE(32640)
#define IDC_ICON        MAKEINTRESOURCE(32641)
#define IDC_SIZENWSE    MAKEINTRESOURCE(32642)
#define IDC_SIZENESW    MAKEINTRESOURCE(32643)
#define IDC_SIZEWE      MAKEINTRESOURCE(32644)
#define IDC_SIZENS      MAKEINTRESOURCE(32645)
#define IDC_SIZEALL     MAKEINTRESOURCE(32646)
#define IDC_NO          MAKEINTRESOURCE(32648)
#define IDC_APPSTARTING MAKEINTRESOURCE(32650)
#define IDC_HELP        MAKEINTRESOURCE(32651)

#define WM_SETCURSOR        0x0020



/* Icon management *************************************************/

#ifndef NOAPIPROTO  /* Prototypes */

HICON   WINAPI LoadIcon(HINSTANCE, LPCSTR);
HICON   WINAPI CreateIcon(HINSTANCE, int, int, BYTE, BYTE, const void FAR*, const void FAR*);
BOOL    WINAPI DestroyIcon(HICON);

#if !defined(TWIN32)
HICON   WINAPI CopyIcon(HINSTANCE, HICON);
#else
HICON   WINAPI CopyIcon(HICON);
#endif

BOOL    WINAPI DrawIcon(HDC, int, int, HICON);
HICON   WINAPI CreateIconFromResource(PBYTE, DWORD, BOOL, DWORD);
HICON   WINAPI CreateIconIndirect(PICONINFO);
BOOL    WINAPI GetIconInfo(HICON, PICONINFO);

#endif              /* NOAPIPROTO */


#ifndef NOICONS

/* Standard icon resource IDs */
#define IDI_APPLICATION     MAKEINTRESOURCE(32512)
#define IDI_HAND        MAKEINTRESOURCE(32513)
#define IDI_QUESTION        MAKEINTRESOURCE(32514)
#define IDI_EXCLAMATION     MAKEINTRESOURCE(32515)
#define IDI_ASTERISK        MAKEINTRESOURCE(32516)

#endif  /* NOICONS */



/* Message box management ******************************************/
#ifndef NOMB

#ifndef NOAPIPROTO  /* Prototypes */

int     WINAPI MessageBox(HWND, LPCSTR, LPCSTR, UINT);

#ifdef	TWIN32
BOOL	WINAPI MessageBeep(UINT);
#else
void	WINAPI MessageBeep(UINT);
#endif

#endif              /* NOAPIPROTO */

#define MB_TYPEMASK		0x000F
#define MB_ICONMASK		0x00F0
#define MB_DEFMASK		0x0F00
#define MB_MODEMASK		0x00003000L
#define MB_MISCMASK		0x0000C000L

#define MB_OK			0x0000
#define MB_OKCANCEL		0x0001
#define MB_ABORTRETRYIGNORE	0x0002
#define MB_YESNOCANCEL		0x0003
#define MB_YESNO		0x0004
#define MB_RETRYCANCEL		0x0005

#define MB_ICONHAND		0x0010
#define MB_ICONQUESTION		0x0020
#define MB_ICONEXCLAMATION	0x0030
#define MB_ICONASTERISK		0x0040
#if	(WINVER >= 0x0400)
#define MB_ICONERROR		MB_ICONHAND
#define MB_ICONWARNING		MB_ICONEXCLAMATION
#define MB_USERICON		0x00000080L
#endif	/* WINVER >= 0x0400 */

#define MB_ICONSTOP		MB_ICONHAND
#define MB_ICONINFORMATION	MB_ICONASTERISK

#define MB_DEFBUTTON1		0x0000
#define MB_DEFBUTTON2		0x0100
#define MB_DEFBUTTON3		0x0200
#if	(WINVER >= 0x0400)
#define MB_DEFBUTTON4		0x00000300L
#endif	/* WINVER >= 0x0400 */

#define MB_APPLMODAL		0x0000
#define MB_SYSTEMMODAL		0x1000
#define MB_TASKMODAL		0x2000
#if	(WINVER >= 0x0400)
#define MB_HELP			0x00004000L
#define MB_RIGHT		0x00080000L
#define MB_RTLREADING		0x00100000L
#endif	/* WINVER >= 0x0400 */

#define MB_NOFOCUS		0x00008000L
#define MB_SETFOREGROUND	0x00010000L
#define MB_DEFAULT_DESKTOP_ONLY	0x00020000L
#define MB_SERVICE_NOTIFICATION	0x00040000L

#endif      /* NOMB */

/* Caret management ************************************************/

#ifndef NOAPIPROTO  /* Prototypes */

void    WINAPI CreateCaret(HWND, HBITMAP, int, int);
void    WINAPI DestroyCaret(void);

void    WINAPI SetCaretPos(int, int);
void    WINAPI GetCaretPos(POINT FAR*);

void    WINAPI HideCaret(HWND);
void    WINAPI ShowCaret(HWND);

UINT    WINAPI GetCaretBlinkTime(void);
void    WINAPI SetCaretBlinkTime(UINT);

#endif              /* NOAPIPROTO */



/* WM_SYSCOMMAND support *******************************************/
#define WM_SYSCOMMAND   0x0112

#ifndef NOSYSCOMMANDS

/* System Menu Command Values */
#define SC_SIZE     0xF000
#define SC_MOVE     0xF010
#define SC_MINIMIZE 0xF020
#define SC_MAXIMIZE 0xF030
#define SC_NEXTWINDOW   0xF040
#define SC_PREVWINDOW   0xF050
#define SC_CLOSE    0xF060
#define SC_VSCROLL  0xF070
#define SC_HSCROLL  0xF080
#define SC_MOUSEMENU    0xF090
#define SC_KEYMENU  0xF100
#define SC_ARRANGE  0xF110
#define SC_RESTORE  0xF120
#define SC_TASKLIST 0xF130
#define SC_SCREENSAVE   0xF140
#define SC_HOTKEY       0xF150

/* Obsolete names */
#define SC_ICON     SC_MINIMIZE
#define SC_ZOOM     SC_MAXIMIZE


#endif  /* NOSYSCOMMANDS */



/* MDI support *****************************************************/
#ifndef NOMDI

/* CreateWindow lpParams structure for creating MDI client */
typedef struct tagCLIENTCREATESTRUCT
{
    HMENU  hWindowMenu;
    UINT   idFirstChild;
} CLIENTCREATESTRUCT;
typedef CLIENTCREATESTRUCT FAR* LPCLIENTCREATESTRUCT;

/* WM_MDICREATE message structure */
typedef struct tagMDICREATESTRUCT
{
    LPCSTR  szClass;
    LPCSTR  szTitle;
    HINSTANCE hOwner;
    int     x;
    int     y;
    int     cx;
    int     cy;
    DWORD   style;
    LPARAM  lParam;
} MDICREATESTRUCT;
typedef MDICREATESTRUCT FAR*  LPMDICREATESTRUCT;


#ifndef NOAPIPROTO  /* Prototypes */

LRESULT WINAPI DefFrameProc(HWND, HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI DefMDIChildProc(HWND, UINT, WPARAM, LPARAM);

#ifndef NOMSG
BOOL    WINAPI TranslateMDISysAccel(HWND, MSG FAR*);
#endif

UINT    WINAPI ArrangeIconicWindows(HWND);

#endif              /* NOAPIPROTO */


#define WM_CHILDACTIVATE    0x0022

/* MDI client style bits */
#define MDIS_ALLCHILDSTYLES 0x0001

/* MDI messages */
#define WM_MDICREATE        0x0220
#define WM_MDIDESTROY       0x0221
#define WM_MDIACTIVATE      0x0222
#define WM_MDIRESTORE       0x0223
#define WM_MDINEXT      0x0224
#define WM_MDIMAXIMIZE      0x0225
#define WM_MDITILE      0x0226
#define WM_MDICASCADE       0x0227
#define WM_MDIICONARRANGE   0x0228
#define WM_MDIGETACTIVE     0x0229
#define WM_MDISETMENU       0x0230
#define WM_ENTERSIZEMOVE    0x0231
#define WM_EXITSIZEMOVE     0x0232
#define WM_DROPFILES        0x0233
#define WM_MDIREFRESHMENU   0x0234

/* wParam values for WM_MDITILE and WM_MDICASCADE messages. */
#define MDITILE_VERTICAL    0x0000
#define MDITILE_HORIZONTAL  0x0001
#define MDITILE_SKIPDISABLED    0x0002


#endif      /* NOMDI */



/* Dialog and control management ***********************************/

/* Dialog notification messages */

#define WM_INITDIALOG       0x0110
#define WM_NEXTDLGCTL       0x0028
#define WM_PARENTNOTIFY     0x0210
#define WM_ENTERIDLE        0x0121

/* Standard dialog button IDs */

#define IDOK            1
#define IDCANCEL        2
#define IDABORT         3
#define IDRETRY         4
#define IDIGNORE        5
#define IDYES           6
#define IDNO            7

/* Standard dialog button IDs (WINVER >= 0x0400) */

#define IDCLOSE		8
#define IDHELP		9

/* Dialog callback */

#ifndef NOCTLMGR

#ifdef STRICT
typedef BOOL (CALLBACK* DLGPROC)(HWND, UINT, WPARAM, LPARAM);
#else
typedef FARPROC DLGPROC;
#endif

#ifndef NOAPIPROTO  /* Prototypes */

#ifndef NOMSG
BOOL    WINAPI IsDialogMessage(HWND, MSG FAR*);
#endif

LRESULT WINAPI DefDlgProc(HWND, UINT, WPARAM, LPARAM);

HWND    WINAPI CreateDialog(HINSTANCE, LPCSTR, HWND, DLGPROC);
HWND    WINAPI CreateDialogIndirect(HINSTANCE, const void FAR*, HWND, DLGPROC);
HWND    WINAPI CreateDialogParam(HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM);
HWND    WINAPI CreateDialogIndirectParam(HINSTANCE, const void FAR*, HWND, DLGPROC, LPARAM);

int     WINAPI DialogBox(HINSTANCE, LPCSTR, HWND, DLGPROC);

#ifdef TWIN32
int     WINAPI DialogBoxIndirect(HINSTANCE, LPDLGTEMPLATE, HWND, DLGPROC);
#else
int	WINAPI DialogBoxIndirect(HINSTANCE, HGLOBAL, HWND, DLGPROC);
#endif

int     WINAPI DialogBoxParam(HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM);

#ifdef TWIN32
int     WINAPI DialogBoxIndirectParam(HINSTANCE, LPDLGTEMPLATE, HWND, DLGPROC, LPARAM);
#else
int	WINAPI DialogBoxIndirectParam(HINSTANCE, HGLOBAL, HWND, DLGPROC, LPARAM);
#endif

#if defined (TWIN32)
BOOL WINAPI
#else
void WINAPI
#endif
EndDialog(HWND, int);

int     WINAPI GetDlgCtrlID(HWND);
HWND    WINAPI GetDlgItem(HWND, int);
LRESULT WINAPI SendDlgItemMessage(HWND, int, UINT, WPARAM, LPARAM);

void    WINAPI SetDlgItemInt(HWND, int, UINT, BOOL);
UINT    WINAPI GetDlgItemInt(HWND, int, BOOL FAR* , BOOL);

void    WINAPI SetDlgItemText(HWND, int, LPCSTR);
int     WINAPI GetDlgItemText(HWND, int, LPSTR, int);

void    WINAPI CheckDlgButton(HWND, int, UINT);
void    WINAPI CheckRadioButton(HWND, int, int, int);
UINT    WINAPI IsDlgButtonChecked(HWND, int);

HWND    WINAPI GetNextDlgGroupItem(HWND, HWND, BOOL);
HWND    WINAPI GetNextDlgTabItem(HWND, HWND, BOOL);

void    WINAPI MapDialogRect(HWND, RECT FAR*);
DWORD   WINAPI GetDialogBaseUnits(void);

#endif      /* NOAPIPROTO */


/* Dialog window class */
#define WC_DIALOG   (MAKEINTATOM(0x8002))

/* cbWndExtra bytes needed by dialog manager for dialog classes */
#define DLGWINDOWEXTRA  4*sizeof(LRESULT)+5*sizeof(WORD) /* 30 (orig MS) */

/* Dialog styles */
#define DS_ABSALIGN		0x0001L
#define DS_SYSMODAL		0x0002L
#define DS_LOCALEDIT		0x0020L
#define DS_SETFONT		0x0040L
#define DS_MODALFRAME		0x0080L
#define DS_NOIDLEMSG		0x0100L
#define DS_SETFOREGROUND	0x0200L  /* not in win3.1 */

/* Dialog styles (WINVER >= 0x0400) */

#define DS_3DLOOK		0x0004L
#define DS_FIXEDSYS		0x0008L
#define DS_NOFAILCREATE		0x0010L
#define DS_CONTROL		0x0400L
#define DS_CENTER		0x0800L
#define DS_CENTERMOUSE		0x1000L
#define DS_CONTEXTHELP		0x2000L

/* Dialog messages */
#define DM_GETDEFID     (WM_USER+0)
#define DM_SETDEFID     (WM_USER+1)

/* Returned in HIWORD() of DM_GETDEFID result if msg is supported */
#define DC_HASDEFID     0x534B

/* Get/SetWindowWord/Long offsets for use with WC_DIALOG windows */
#define DWL_MSGRESULT   0
#define DWL_DLGPROC	DWL_MSGRESULT+sizeof(LRESULT)
#define DWL_USER	DWL_DLGPROC+sizeof(LRESULT)

#define WM_GETDLGCODE       0x0087

/* dialog codes */
#define DLGC_WANTARROWS     0x0001
#define DLGC_WANTTAB        0x0002
#define DLGC_WANTALLKEYS    0x0004
#define DLGC_WANTMESSAGE    0x0004
#define DLGC_HASSETSEL      0x0008
#define DLGC_DEFPUSHBUTTON  0x0010
#define DLGC_UNDEFPUSHBUTTON 0x0020
#define DLGC_RADIOBUTTON    0x0040
#define DLGC_WANTCHARS      0x0080
#define DLGC_STATIC         0x0100
#define DLGC_BUTTON         0x2000

/* WM_CTLCOLOR control IDs */
#define CTLCOLOR_MSGBOX     0
#define CTLCOLOR_EDIT       1
#define CTLCOLOR_LISTBOX    2
#define CTLCOLOR_BTN        3
#define CTLCOLOR_DLG        4
#define CTLCOLOR_SCROLLBAR  5
#define CTLCOLOR_STATIC     6

#ifndef	TWIN32
#define WM_CTLCOLOR     0x0019
#else
#define	WM_CTLCOLORMSGBOX	0x0132
#define	WM_CTLCOLOREDIT		0x0133
#define	WM_CTLCOLORLISTBOX	0x0134
#define	WM_CTLCOLORBTN		0x0135
#define	WM_CTLCOLORDLG		0x0136
#define	WM_CTLCOLORSCROLLBAR	0x0137
#define	WM_CTLCOLORSTATIC	0x0138
#endif

#define WM_SETFONT          0x0030
#define WM_GETFONT      0x0031


#endif      /* NOCTLMGR */




/* Owner draw control support **************************************/

typedef struct tagDRAWITEMSTRUCT
{
    UINT        CtlType;
    UINT        CtlID;
    UINT        itemID;
    UINT        itemAction;
    UINT        itemState;
    HWND    hwndItem;
    HDC     hDC;
    RECT    rcItem;
    DWORD       itemData;
} DRAWITEMSTRUCT;
typedef DRAWITEMSTRUCT NEAR* PDRAWITEMSTRUCT;
typedef DRAWITEMSTRUCT FAR* LPDRAWITEMSTRUCT;

typedef struct tagMEASUREITEMSTRUCT
{
    UINT        CtlType;
    UINT        CtlID;
    UINT        itemID;
    UINT        itemWidth;
    UINT        itemHeight;
    DWORD       itemData;
} MEASUREITEMSTRUCT;
typedef MEASUREITEMSTRUCT NEAR* PMEASUREITEMSTRUCT;
typedef MEASUREITEMSTRUCT FAR* LPMEASUREITEMSTRUCT;

typedef struct tagDELETEITEMSTRUCT
{
    UINT       CtlType;
    UINT       CtlID;
    UINT       itemID;
    HWND       hwndItem;
    DWORD      itemData;
} DELETEITEMSTRUCT;
typedef DELETEITEMSTRUCT NEAR* PDELETEITEMSTRUCT;
typedef DELETEITEMSTRUCT FAR* LPDELETEITEMSTRUCT;

typedef struct tagCOMPAREITEMSTRUCT
{
    UINT        CtlType;
    UINT        CtlID;
    HWND    hwndItem;
    UINT        itemID1;
    DWORD       itemData1;
    UINT        itemID2;
    DWORD       itemData2;
} COMPAREITEMSTRUCT;
typedef COMPAREITEMSTRUCT NEAR* PCOMPAREITEMSTRUCT;
typedef COMPAREITEMSTRUCT FAR* LPCOMPAREITEMSTRUCT;


#define WM_DRAWITEM         0x002B
#define WM_MEASUREITEM      0x002C
#define WM_DELETEITEM       0x002D
#define WM_COMPAREITEM      0x0039

/* Owner draw control types */
#define ODT_MENU    1
#define ODT_LISTBOX 2
#define ODT_COMBOBOX    3
#define ODT_BUTTON  4

/* Owner draw actions */
#define ODA_DRAWENTIRE  0x0001
#define ODA_SELECT  0x0002
#define ODA_FOCUS   0x0004

/* Owner draw state */
#define ODS_SELECTED    0x0001
#define ODS_GRAYED  0x0002
#define ODS_DISABLED    0x0004
#define ODS_CHECKED 0x0008
#define ODS_FOCUS   0x0010



/* Static control support ******************************************/

#ifndef NOCTLMGR

/* Static Control Styles */
#define SS_LEFT             0x00000000L
#define SS_CENTER           0x00000001L
#define SS_RIGHT            0x00000002L
#define SS_ICON             0x00000003L
#define SS_BLACKRECT        0x00000004L
#define SS_GRAYRECT         0x00000005L
#define SS_WHITERECT        0x00000006L
#define SS_BLACKFRAME       0x00000007L
#define SS_GRAYFRAME        0x00000008L
#define SS_WHITEFRAME       0x00000009L
#define SS_SIMPLE           0x0000000BL
#define SS_LEFTNOWORDWRAP   0x0000000CL
#define SS_BITMAP	    0X0000000EL
#define SS_NOPREFIX         0x00000080L

/* Static Control Mesages */

#ifndef NOWINMESSAGES

#if 0	/* Old Windows 3.x values */
#define STM_SETICON     (WM_USER+0)
#define STM_GETICON     (WM_USER+1)
#endif

#define STM_SETICON	0x0170
#define STM_GETICON	0x0171
#define STM_SETIMAGE	0x0172		/* NOT IMPLEMENTED */
#define STM_GETIMAGE	0x0173		/* NOT IMPLEMENTED */
#define STM_MSGMAX	0x0174		/* NOT IMPLEMENTED */

	/* WINVER >= 0x0400 */
#define STN_CLICKED         0		/* NOT IMPLEMENTED */
#define STN_DBLCLK          1		/* NOT IMPLEMENTED */
#define STN_ENABLE          2		/* NOT IMPLEMENTED */
#define STN_DISABLE         3		/* NOT IMPLEMENTED */

#endif  /* NOWINMESSAGES */

#endif      /* NOCTLMGR */

/* Button control support ******************************************/

#ifndef NOCTLMGR

/* Button Control Styles */
#define BS_PUSHBUTTON       0x00000000L
#define BS_DEFPUSHBUTTON    0x00000001L
#define BS_CHECKBOX         0x00000002L
#define BS_AUTOCHECKBOX     0x00000003L
#define BS_RADIOBUTTON      0x00000004L
#define BS_3STATE           0x00000005L
#define BS_AUTO3STATE       0x00000006L
#define BS_GROUPBOX         0x00000007L
#define BS_USERBUTTON       0x00000008L
#define BS_AUTORADIOBUTTON  0x00000009L
#define BS_OWNERDRAW        0x0000000BL
#define BS_LEFTTEXT         0x00000020L
#define BS_TEXT             0x00000000L

#if defined (TWIN32) /* if (WINVER >= 0x0400) */
#define BS_ICON             0x00000040L
#define BS_BITMAP           0x00000080L
#define BS_LEFT             0x00000100L
#define BS_RIGHT            0x00000200L
#define BS_CENTER           0x00000300L
#define BS_TOP              0x00000400L
#define BS_BOTTOM           0x00000800L
#define BS_VCENTER          0x00000C00L
#define BS_PUSHLIKE         0x00001000L
#define BS_MULTILINE        0x00002000L
#define BS_NOTIFY           0x00004000L
#define BS_FLAT             0x00008000L
#define BS_RIGHTBUTTON      BS_LEFTTEXT
#endif

/* Button Control Messages  */

#if 0	/* Windows 3.X values */
#define BM_GETCHECK     (WM_USER+0)
#define BM_SETCHECK     (WM_USER+1)
#define BM_GETSTATE     (WM_USER+2)
#define BM_SETSTATE     (WM_USER+3)
#define BM_SETSTYLE     (WM_USER+4)
#endif

	/* Windows 4.0 values */
#define BM_GETCHECK		0x00F0
#define BM_SETCHECK		0x00F1
#define BM_GETSTATE		0x00F2
#define BM_SETSTATE		0x00F3
#define BM_SETSTYLE		0x00F4

	/* if (WINVER >= 0x0400) */
#define BM_CLICK		0x00F5		/* NOT IMPLEMENTED */
#define BM_GETIMAGE		0x00F6		/* NOT IMPLEMENTED */
#define BM_SETIMAGE		0x00F7		/* NOT IMPLEMENTED */
#define BST_UNCHECKED		0x0000		/* NOT IMPLEMENTED */
#define BST_CHECKED		0x0001		/* NOT IMPLEMENTED */
#define BST_INDETERMINATE	0x0002		/* NOT IMPLEMENTED */
#define BST_PUSHED		0x0004		/* NOT IMPLEMENTED */
#define BST_FOCUS		0x0008		/* NOT IMPLEMENTED */
	/* endif (WINVER >= 0x0400) */

/* User Button Notification Codes */
#define BN_CLICKED          0
#define BN_PAINT            1
#define BN_HILITE           2
#define BN_UNHILITE         3
#define BN_DISABLE          4
#define BN_DOUBLECLICKED    5

#endif      /* NOCTLMGR */

/* Edit control support ********************************************/
#ifndef NOCTLMGR

typedef int
(CALLBACK *EDITWORDBREAKPROC)(LPSTR lpch, int ichCurrent, int cch, int code);

#ifndef NOWINSTYLES

/* Edit control styles */
#define ES_LEFT             0x00000000L
#define ES_CENTER           0x00000001L
#define ES_RIGHT            0x00000002L
#define ES_MULTILINE        0x00000004L
#define ES_UPPERCASE        0x00000008L
#define ES_LOWERCASE        0x00000010L
#define ES_PASSWORD         0x00000020L
#define ES_AUTOVSCROLL      0x00000040L
#define ES_AUTOHSCROLL      0x00000080L
#define ES_NOHIDESEL        0x00000100L
#define ES_OEMCONVERT       0x00000400L
#define ES_READONLY         0x00000800L
#define ES_WANTRETURN       0x00001000L

#endif  /* NOWINSTYLES */


#ifndef NOWINMESSAGES

/* Edit control messages */
#define EM_BASE		WM_USER
#define EM_GETSEL               (EM_BASE+0)
#define EM_SETSEL               (EM_BASE+1)
#define EM_GETRECT              (EM_BASE+2)
#define EM_SETRECT              (EM_BASE+3)
#define EM_SETRECTNP            (EM_BASE+4)
#define EM_SCROLL               (EM_BASE+5)
#define EM_LINESCROLL           (EM_BASE+6)
#define EM_SCROLLCARET          (EM_BASE+7)
#define EM_GETMODIFY            (EM_BASE+8)
#define EM_SETMODIFY            (EM_BASE+9)
#define EM_GETLINECOUNT         (EM_BASE+10)
#define EM_LINEINDEX            (EM_BASE+11)
#define EM_SETHANDLE            (EM_BASE+12)
#define EM_GETHANDLE            (EM_BASE+13)
#define EM_LINELENGTH           (EM_BASE+17)
#define EM_REPLACESEL           (EM_BASE+18)
#define EM_SETFONT              (EM_BASE+19)    /* NOT IMPLEMENTED: use WM_SETFONT */
#define EM_GETLINE              (EM_BASE+20)
#define EM_LIMITTEXT            (EM_BASE+21)
#define EM_CANUNDO              (EM_BASE+22)
#define EM_UNDO                 (EM_BASE+23)
#define EM_FMTLINES             (EM_BASE+24)
#define EM_LINEFROMCHAR         (EM_BASE+25)
#define EM_SETWORDBREAK         (EM_BASE+26)    /* NOT IMPLEMENTED: use EM_SETWORDBREAK */
#define EM_SETTABSTOPS          (EM_BASE+27)
#define EM_SETPASSWORDCHAR      (EM_BASE+28)
#define EM_EMPTYUNDOBUFFER      (EM_BASE+29)
#define EM_GETFIRSTVISIBLELINE  (EM_BASE+30)
#define EM_SETREADONLY          (EM_BASE+31)
#define EM_SETWORDBREAKPROC     (EM_BASE+32)
#define EM_GETWORDBREAKPROC     (EM_BASE+33)
#define EM_GETPASSWORDCHAR      (EM_BASE+34)

	/* #if (WINVER >= 0x0400) */
#define EM_SETMARGINS		0x00D3		/* NOT IMPLEMENTED */
#define EM_GETMARGINS		0x00D4		/* NOT IMPLEMENTED */
#define EM_SETLIMITTEXT		EM_LIMITTEXT	/* win40 name change */
#define EM_GETLIMITTEXT		0x00D5		/* NOT IMPLEMENTED */
#define EM_POSFROMCHAR		0x00D6		/* NOT IMPLEMENTED */
#define EM_CHARFROMPOS		0x00D7		/* NOT IMPLEMENTED */
	/* #endif (WINVER >= 0x0400) */

#endif  /* NOWINMESSAGES */

/* EDITWORDBREAKPROC code values */
#define WB_LEFT        0
#define WB_RIGHT       1
#define WB_ISDELIMITER     2

/* Edit control notification codes */
#define EN_SETFOCUS     0x0100
#define EN_KILLFOCUS        0x0200
#define EN_CHANGE       0x0300
#define EN_UPDATE       0x0400
#define EN_ERRSPACE     0x0500
#define EN_MAXTEXT      0x0501
#define EN_HSCROLL      0x0601
#define EN_VSCROLL      0x0602

/* Edit control EM_SETMARGIN parameters */

	/* #if (WINVER >= 0x0400) */
#define EC_LEFTMARGIN	0x0001			/* NOT IMPLEMENTED */
#define EC_RIGHTMARGIN	0x0002			/* NOT IMPLEMENTED */
#define EC_USEFONTINFO	0xffff			/* NOT IMPLEMENTED */
	/* #endif (WINVER >= 0x0400) */

#endif      /* NOCTLMGR */



/* Scroll bar control support **************************************/
#ifndef NOCTLMGR

/* Scroll bar styles */

#ifndef NOWINSTYLES

#define SBS_HORZ			0x0000L
#define SBS_VERT			0x0001L
#define SBS_TOPALIGN			0x0002L
#define SBS_LEFTALIGN			0x0002L
#define SBS_BOTTOMALIGN			0x0004L
#define SBS_RIGHTALIGN			0x0004L
#define SBS_SIZEBOXTOPLEFTALIGN		0x0002L
#define SBS_SIZEBOXBOTTOMRIGHTALIGN	0x0004L
#define SBS_SIZEBOX			0x0008L

#endif  /* NOWINSTYLES */

/* Scroll bar styles (WINVER >= 0x0400) */

#ifndef NOWINSTYLES

#define SBS_SIZEGRIP			0x0010L		/* NOT IMPLEMENTED */

#endif  /* NOWINSTYLES */

#endif	/* NOCTLMGR */

/* Listbox control support *****************************************/

#ifndef NOCTLMGR

#ifndef NOWINSTYLES

/* Listbox styles */
#define LBS_NOTIFY        0x0001L
#define LBS_SORT          0x0002L
#define LBS_NOREDRAW          0x0004L
#define LBS_MULTIPLESEL       0x0008L
#define LBS_OWNERDRAWFIXED    0x0010L
#define LBS_OWNERDRAWVARIABLE 0x0020L
#define LBS_HASSTRINGS        0x0040L
#define LBS_USETABSTOPS       0x0080L
#define LBS_NOINTEGRALHEIGHT  0x0100L
#define LBS_MULTICOLUMN       0x0200L
#define LBS_WANTKEYBOARDINPUT 0x0400L
#define LBS_EXTENDEDSEL       0x0800L
#define LBS_DISABLENOSCROLL   0x1000L
#define LBS_STANDARD          (LBS_NOTIFY | LBS_SORT | WS_VSCROLL | WS_BORDER)

#endif  /* NOWINSTYLES */


#ifndef NOWINMESSAGES

/* Listbox messages */

#define LB_ADDSTRING           (WM_USER+1)
#define LB_INSERTSTRING        (WM_USER+2)
#define LB_DELETESTRING        (WM_USER+3)
#define LB_SELITEMRANGEEX	0x0183		/* NOT IMPLEMENTED */
#define LB_RESETCONTENT        (WM_USER+5)
#define LB_SETSEL              (WM_USER+6)
#define LB_SETCURSEL           (WM_USER+7)
#define LB_GETSEL              (WM_USER+8)
#define LB_GETCURSEL           (WM_USER+9)
#define LB_GETTEXT             (WM_USER+10)
#define LB_GETTEXTLEN          (WM_USER+11)
#define LB_GETCOUNT            (WM_USER+12)
#define LB_SELECTSTRING        (WM_USER+13)
#define LB_DIR                 (WM_USER+14)
#define LB_GETTOPINDEX         (WM_USER+15)
#define LB_FINDSTRING          (WM_USER+16)
#define LB_GETSELCOUNT         (WM_USER+17)
#define LB_GETSELITEMS         (WM_USER+18)
#define LB_SETTABSTOPS         (WM_USER+19)
#define LB_GETHORIZONTALEXTENT (WM_USER+20)
#define LB_SETHORIZONTALEXTENT (WM_USER+21)
#define LB_SETCOLUMNWIDTH      (WM_USER+22)
#define LB_ADDFILE		0x0196		/* NOT IMPLEMENTED */
#define LB_SETTOPINDEX         (WM_USER+24)
#define LB_GETITEMRECT         (WM_USER+25)
#define LB_GETITEMDATA         (WM_USER+26)
#define LB_SETITEMDATA         (WM_USER+27)
#define LB_SELITEMRANGE        (WM_USER+28)
#define LB_SETANCHORINDEX	0x019C		/* NOT IMPLEMENTED */
#define LB_GETANCHORINDEX	0x019D		/* NOT IMPLEMENTED */
#define LB_SETCARETINDEX       (WM_USER+31)
#define LB_GETCARETINDEX       (WM_USER+32)
#define LB_SETITEMHEIGHT       (WM_USER+33)
#define LB_GETITEMHEIGHT       (WM_USER+34)
#define LB_FINDSTRINGEXACT     (WM_USER+35)
#define LB_SETLOCALE		0x01A5		/* NOT IMPLEMENTED */
#define LB_GETLOCALE		0x01A6		/* NOT IMPLEMENTED */
#define LB_SETCOUNT		0x01A7		/* NOT IMPLEMENTED */
#define LB_INITSTORAGE		0x01A8		/* NOT IMPLEMENTED */
#define LB_ITEMFROMPOINT	0x01A9		/* NOT IMPLEMENTED */

#if	(WINVER >= 0x0400)
#define LB_MSGMAX		0x01B0
#else
#define LB_MSGMAX		0x01A8
#endif

#endif  /* NOWINMESSAGES */

/* Listbox notification codes */
#define LBN_ERRSPACE        (-2)
#define LBN_SELCHANGE       1
#define LBN_DBLCLK      2
#define LBN_SELCANCEL       3
#define LBN_SETFOCUS        4
#define LBN_KILLFOCUS       5

/* Listbox notification messages */
#define WM_VKEYTOITEM       0x002E
#define WM_CHARTOITEM       0x002F

/* Listbox message return values */
#define LB_OKAY         0
#define LB_ERR          (-1)
#define LB_ERRSPACE     (-2)

#define LB_CTLCODE      0L


#endif      /* NOCTLMGR */



/* Dialog directory support ****************************************/
#ifndef NOCTLMGR

#ifndef NOAPIPROTO  /* Prototypes */

int     WINAPI DlgDirList(HWND, LPSTR, int, int, UINT);
BOOL    WINAPI DlgDirSelect(HWND, LPSTR, int);

int     WINAPI DlgDirListComboBox(HWND, LPSTR, int, int, UINT);
BOOL    WINAPI DlgDirSelectComboBox(HWND, LPSTR, int);

BOOL    WINAPI DlgDirSelectEx(HWND, LPSTR, int, int);
BOOL    WINAPI DlgDirSelectComboBoxEx(HWND, LPSTR, int, int);

#endif              /* NOAPIPROTO */


/* DlgDirList, DlgDirListComboBox flags values */
#define DDL_READWRITE       0x0000
#define DDL_READONLY        0x0001
#define DDL_HIDDEN          0x0002
#define DDL_SYSTEM          0x0004
#define DDL_DIRECTORY       0x0010
#define DDL_ARCHIVE     0x0020

#define DDL_POSTMSGS        0x2000
#define DDL_DRIVES      0x4000
#define DDL_EXCLUSIVE       0x8000


#endif      /* NOCTLMGR */



/* Combo box control support ***************************************/
#ifndef NOCTLMGR

#ifndef NOWINSTYLES

/* Combo box styles */
#define CBS_SIMPLE            0x0001L
#define CBS_DROPDOWN          0x0002L
#define CBS_DROPDOWNLIST      0x0003L
#define CBS_OWNERDRAWFIXED    0x0010L
#define CBS_OWNERDRAWVARIABLE 0x0020L
#define CBS_AUTOHSCROLL       0x0040L
#define CBS_OEMCONVERT        0x0080L
#define CBS_SORT              0x0100L
#define CBS_HASSTRINGS        0x0200L
#define CBS_NOINTEGRALHEIGHT  0x0400L
#define CBS_DISABLENOSCROLL   0x0800L

#endif  /* NOWINSTYLES */


#ifndef NOWINMESSAGES

/* Combo box messages */
#define CB_GETEDITSEL            (WM_USER+0)
#define CB_LIMITTEXT             (WM_USER+1)
#define CB_SETEDITSEL            (WM_USER+2)
#define CB_ADDSTRING             (WM_USER+3)
#define CB_DELETESTRING          (WM_USER+4)
#define CB_DIR                   (WM_USER+5)
#define CB_GETCOUNT              (WM_USER+6)
#define CB_GETCURSEL             (WM_USER+7)
#define CB_GETLBTEXT             (WM_USER+8)
#define CB_GETLBTEXTLEN          (WM_USER+9)
#define CB_INSERTSTRING          (WM_USER+10)
#define CB_RESETCONTENT          (WM_USER+11)
#define CB_FINDSTRING            (WM_USER+12)
#define CB_SELECTSTRING          (WM_USER+13)
#define CB_SETCURSEL             (WM_USER+14)
#define CB_SHOWDROPDOWN          (WM_USER+15)
#define CB_GETITEMDATA           (WM_USER+16)
#define CB_SETITEMDATA           (WM_USER+17)
#define CB_GETDROPPEDCONTROLRECT (WM_USER+18)
#define CB_SETITEMHEIGHT         (WM_USER+19)
#define CB_GETITEMHEIGHT         (WM_USER+20)
#define CB_SETEXTENDEDUI         (WM_USER+21)
#define CB_GETEXTENDEDUI         (WM_USER+22)
#define CB_GETDROPPEDSTATE       (WM_USER+23)
#define CB_FINDSTRINGEXACT       (WM_USER+24)

#define CB_SETLOCALE		0x0159		/* NOT IMPLEMENTED */
#define CB_GETLOCALE		0x015A		/* NOT IMPLEMENTED */
	/* #if (WINVER >= 0x0400) */
#define CB_GETTOPINDEX		0x015b		/* NOT IMPLEMENTED */
#define CB_SETTOPINDEX		0x015c		/* NOT IMPLEMENTED */
#define CB_GETHORIZONTALEXTENT	0x015d		/* NOT IMPLEMENTED */
#define CB_SETHORIZONTALEXTENT	0x015e		/* NOT IMPLEMENTED */
#define CB_GETDROPPEDWIDTH	0x015f		/* NOT IMPLEMENTED */
#define CB_SETDROPPEDWIDTH	0x0160		/* NOT IMPLEMENTED */
#define CB_INITSTORAGE		0x0161		/* NOT IMPLEMENTED */
	/* #endif (WINVER >= 0x0400) */

#if	(WINVER >= 0x0400)
#define CB_MSGMAX                   0x0162
#else
#define CB_MSGMAX                   0x015B
#endif

#endif  /* NOWINMESSAGES */


/* Combo box notification codes */
#define CBN_ERRSPACE        (-1)
#define CBN_SELCHANGE       1
#define CBN_DBLCLK          2
#define CBN_SETFOCUS        3
#define CBN_KILLFOCUS       4
#define CBN_EDITCHANGE      5
#define CBN_EDITUPDATE      6
#define CBN_DROPDOWN        7
#define CBN_CLOSEUP         8
#define CBN_SELENDOK        9
#define CBN_SELENDCANCEL    10

/* Combo box message return values */
#define CB_OKAY         0
#define CB_ERR          (-1)
#define CB_ERRSPACE     (-2)


#endif      /* NOCTLMGR */



/* Windows hook support ********************************************/
#ifndef NOWH

DECLARE_HANDLE32(HHOOK);

#ifdef STRICT
typedef LRESULT (CALLBACK* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);
#else
typedef FARPROC HOOKPROC;
#endif


#ifndef NOAPIPROTO  /* Prototypes */

#ifdef STRICT
HHOOK   WINAPI SetWindowsHook(int, HOOKPROC);
LRESULT WINAPI DefHookProc(int, WPARAM, LPARAM, HHOOK FAR*);
#else
HOOKPROC WINAPI SetWindowsHook(int, HOOKPROC);
LRESULT WINAPI DefHookProc(int, WPARAM, LPARAM, HOOKPROC FAR*);
#endif

BOOL    WINAPI UnhookWindowsHook(int, HOOKPROC);

HHOOK   WINAPI SetWindowsHookEx(int idHook, HOOKPROC lpfn, HINSTANCE hInstance, HTASK hTask);
BOOL    WINAPI UnhookWindowsHookEx(HHOOK hHook);
LRESULT WINAPI CallNextHookEx(HHOOK hHook, int code, WPARAM wParam, LPARAM lParam);

#endif              /* NOAPIPROTO */


/* Standard hook code */
#define HC_ACTION       0

/* Obsolete hook codes (NO LONGER SUPPORTED) */
#define HC_GETLPLPFN        (-3)
#define HC_LPLPFNNEXT       (-2)
#define HC_LPFNNEXT     (-1)


#endif      /* NOWH */



/* Debugger support ************************************************/

typedef struct tagDEBUGHOOKINFO
{
    HMODULE hModuleHook;
    LPARAM  reserved;
    LPARAM  lParam;
    WPARAM  wParam;
    int         code;
} DEBUGHOOKINFO;
typedef DEBUGHOOKINFO FAR* LPDEBUGHOOKINFO;


#ifndef NOAPIPROTO  /* Prototypes */

#ifndef NOMSG
BOOL WINAPI QuerySendMessage(HANDLE h1, HANDLE h2, HANDLE h3, LPMSG lpmsg);
#endif  /* NOMSG */

BOOL WINAPI LockInput(HANDLE h1, HWND hwndInput, BOOL fLock);

LONG WINAPI GetSystemDebugState(void);

#endif              /* NOAPIPROTO */


/* SetWindowsHook debug hook support */
#define WH_DEBUG        9

/* Flags returned by GetSystemDebugState. */
#define SDS_MENU        0x0001
#define SDS_SYSMODAL    0x0002
#define SDS_NOTASKQUEUE 0x0004
#define SDS_DIALOG      0x0008
#define SDS_TASKLOCKED  0x0010



/* Help support ****************************************************/

#ifndef NOHELP

typedef struct tagMULTIKEYHELP
{
    UINT    mkSize;
    BYTE    mkKeylist;
    BYTE    szKeyphrase[1];
} MULTIKEYHELP;

typedef struct
{
    UINT wStructSize;
    UINT x;
    UINT y;
    UINT dx;
    UINT dy;
    UINT wMax;
    char rgchMember[2];
} HELPWININFO;
typedef HELPWININFO NEAR* PHELPWININFO;
typedef HELPWININFO FAR* LPHELPWININFO;

typedef struct tagHELPINFO
{
    UINT   cbSize;
    int    iContextType;
    int    iCtrlId;
    HANDLE hItemHandle;
    DWORD  dwContextId;
    POINT  MousePos;
} HELPINFO;
typedef HELPINFO	*PHELPINFO;
typedef HELPINFO NEAR	*NPHELPINFO;
typedef HELPINFO FAR	*LPHELPINFO;

#ifndef NOAPIPROTO  /* Prototypes */

BOOL WINAPI WinHelp(HWND hwndMain, LPCSTR lpszHelp, UINT usCommand, DWORD ulData);

#endif              /* NOAPIPROTO */

/* WinHelp() commands */
#define HELP_CONTEXT      0x0001
#define HELP_QUIT         0x0002
#define HELP_INDEX        0x0003
#define HELP_CONTENTS     0x0003
#define HELP_HELPONHELP   0x0004
#define HELP_SETINDEX     0x0005
#define HELP_SETCONTENTS  0x0005
#define HELP_CONTEXTPOPUP 0x0008
#define HELP_FORCEFILE    0x0009
#define HELP_KEY          0x0101
#define HELP_COMMAND      0x0102
#define HELP_PARTIALKEY   0x0105
#define HELP_MULTIKEY     0x0201
#define HELP_SETWINPOS    0x0203

#define HELP_CONTEXTMENU	0x000a
#define HELP_FINDER		0x000b
#define HELP_WM_HELP		0x000c
#define HELP_SETPOPUP_POS	0x000d

#endif      /* NOHELP */

/* Sound support ***************************************************/

#ifndef NOSOUND

#ifndef NOAPIPROTO  /* Prototypes */

int     WINAPI OpenSound(void);
void    WINAPI CloseSound(void);

int     WINAPI StartSound(void);
int     WINAPI StopSound(void);

int     WINAPI SetVoiceQueueSize(int, int);
int     WINAPI SetVoiceNote(int, int, int, int);
int     WINAPI SetVoiceAccent(int, int, int, int, int);
int     WINAPI SetVoiceEnvelope(int, int, int);
int     WINAPI SetVoiceSound(int, DWORD, int);

int     WINAPI SetVoiceThreshold(int, int);
int FAR* WINAPI GetThresholdEvent(void);
int     WINAPI GetThresholdStatus(void);

int     WINAPI SetSoundNoise(int, int);

int     WINAPI WaitSoundState(int);

int     WINAPI SyncAllVoices(void);
int     WINAPI CountVoiceNotes(int);

#endif              /* NOAPIPROTO */

/* SetSoundNoise() Sources */
#define S_PERIOD512   0
#define S_PERIOD1024  1
#define S_PERIOD2048  2
#define S_PERIODVOICE 3
#define S_WHITE512    4
#define S_WHITE1024   5
#define S_WHITE2048   6
#define S_WHITEVOICE  7

/* WaitSoundState() constants */
#define S_QUEUEEMPTY        0
#define S_THRESHOLD     1
#define S_ALLTHRESHOLD      2

/* Accent Modes */
#define S_NORMAL      0
#define S_LEGATO      1
#define S_STACCATO    2

/* Error return values */
#define S_SERDVNA     (-1)
#define S_SEROFM      (-2)
#define S_SERMACT     (-3)
#define S_SERQFUL     (-4)
#define S_SERBDNT     (-5)
#define S_SERDLN      (-6)
#define S_SERDCC      (-7)
#define S_SERDTP      (-8)
#define S_SERDVL      (-9)
#define S_SERDMD      (-10)
#define S_SERDSH      (-11)
#define S_SERDPT      (-12)
#define S_SERDFQ      (-13)
#define S_SERDDR      (-14)
#define S_SERDSR      (-15)
#define S_SERDST      (-16)

#endif      /* NOSOUND */

/* Comm support ****************************************************/

#ifndef NOCOMM

typedef struct tagDCB
{
    BYTE Id;
    UINT BaudRate;
    BYTE ByteSize;
    BYTE Parity;
    BYTE StopBits;
    UINT RlsTimeout;
    UINT CtsTimeout;
    UINT DsrTimeout;

    UINT fBinary        :1;
    UINT fRtsDisable    :1;
    UINT fParity        :1;
    UINT fOutxCtsFlow   :1;
    UINT fOutxDsrFlow   :1;
    UINT fDummy         :2;
    UINT fDtrDisable    :1;

    UINT fOutX          :1;
    UINT fInX           :1;
    UINT fPeChar        :1;
    UINT fNull          :1;
    UINT fChEvt         :1;
    UINT fDtrflow       :1;
    UINT fRtsflow       :1;
    UINT fDummy2        :1;

    char XonChar;
    char XoffChar;
    UINT XonLim;
    UINT XoffLim;
    char PeChar;
    char EofChar;
    char EvtChar;
    UINT TxDelay;
} DCB;
typedef DCB FAR* LPDCB;

typedef struct tagCOMSTAT
{
    BYTE status;
    UINT cbInQue;
    UINT cbOutQue;
} COMSTAT;

#define CSTF_CTSHOLD    0x01
#define CSTF_DSRHOLD    0x02
#define CSTF_RLSDHOLD   0x04
#define CSTF_XOFFHOLD   0x08
#define CSTF_XOFFSENT   0x10
#define CSTF_EOF        0x20
#define CSTF_TXIM       0x40

#ifndef NOAPIPROTO  /* Prototypes */

int     WINAPI BuildCommDCB(LPCSTR, DCB FAR*);

int     WINAPI OpenComm(LPCSTR, UINT, UINT);
int     WINAPI CloseComm(int);

int     WINAPI ReadComm(int, void FAR*, int);
int     WINAPI WriteComm(int, const void FAR*, int);
int     WINAPI UngetCommChar(int, char);
int     WINAPI FlushComm(int, int);
int     WINAPI TransmitCommChar(int, char);

int     WINAPI SetCommState(const DCB FAR*);
int     WINAPI GetCommState(int, DCB FAR*);
int     WINAPI GetCommError(int, COMSTAT FAR* );

int     WINAPI SetCommBreak(int);
int     WINAPI ClearCommBreak(int);

UINT FAR* WINAPI SetCommEventMask(int, UINT);
UINT    WINAPI GetCommEventMask(int, int);

LONG    WINAPI EscapeCommFunction(int, int);

BOOL    WINAPI EnableCommNotification(int, HWND, int, int);

#endif              /* NOAPIPROTO */

#define WM_COMMNOTIFY       0x0044

#define NOPARITY        0
#define ODDPARITY       1
#define EVENPARITY      2
#define MARKPARITY      3
#define SPACEPARITY     4

#define ONESTOPBIT      0
#define ONE5STOPBITS    1
#define TWOSTOPBITS     2

#define IGNORE              0
/* moved to Win_Base.h
#define INFINITE            0xFFFFFFFF
 */

/* Error Flags */
#define CE_RXOVER           0x0001
#define CE_OVERRUN          0x0002
#define CE_RXPARITY         0x0004
#define CE_FRAME            0x0008
#define CE_BREAK            0x0010
#define CE_CTSTO            0x0020
#define CE_DSRTO            0x0040
#define CE_RLSDTO           0x0080
#define CE_TXFULL           0x0100
#define CE_PTO              0x0200
#define CE_IOE              0x0400
#define CE_DNS              0x0800
#define CE_OOP              0x1000
#define CE_MODE             0x8000

#define IE_BADID            (-1)
#define IE_OPEN             (-2)
#define IE_NOPEN            (-3)
#define IE_MEMORY           (-4)
#define IE_DEFAULT          (-5)
#define IE_HARDWARE         (-10)
#define IE_BYTESIZE         (-11)
#define IE_BAUDRATE         (-12)

/* Events */
#define EV_RXCHAR           0x0001
#define EV_RXFLAG           0x0002
#define EV_TXEMPTY          0x0004
#define EV_CTS              0x0008
#define EV_DSR              0x0010
#define EV_RLSD             0x0020
#define EV_BREAK            0x0040
#define EV_ERR              0x0080
#define EV_RING             0x0100
#define EV_PERR             0x0200
#define EV_CTSS             0x0400
#define EV_DSRS             0x0800
#define EV_RLSDS            0x1000
#define EV_RingTe           0x2000
#define EV_RINGTE       EV_RingTe

/* Escape Functions */
#define SETXOFF             1
#define SETXON              2
#define SETRTS              3
#define CLRRTS              4
#define SETDTR              5
#define CLRDTR              6
#define RESETDEV            7

#define LPTx                0x80

/* new escape functions */
#define GETMAXLPT           8
#define GETMAXCOM           9
#define GETBASEIRQ          10

/* Comm Baud Rate indices */
#define CBR_110      0xFF10
#define CBR_300      0xFF11
#define CBR_600      0xFF12
#define CBR_1200     0xFF13
#define CBR_2400     0xFF14
#define CBR_4800     0xFF15
#define CBR_9600     0xFF16
#define CBR_14400    0xFF17
#define CBR_19200    0xFF18
#define CBR_38400    0xFF1B
#define CBR_56000    0xFF1F
#define CBR_128000   0xFF23
#define CBR_256000   0xFF27

/* notifications passed in low word of lParam on WM_COMMNOTIFY messages */
#define CN_RECEIVE  0x0001
#define CN_TRANSMIT 0x0002
#define CN_EVENT    0x0004

#endif      /* NOCOMM */

/* String formatting support ***************************************/

#ifndef NOAPIPROTO  /* Prototypes */

int     WINAPI wvsprintf(LPSTR lpszOut, LPCSTR lpszFmt, void *lpParams);

int FAR CDECL wsprintf(LPSTR lpszOut, LPCSTR lpszFmt, ...);

#endif              /* NOAPIPROTO */

/* Driver support **************************************************/

#ifndef NODRIVERS

DECLARE_HANDLE(HDRVR);

typedef LRESULT (CALLBACK* DRIVERPROC)(DWORD, HDRVR, UINT, LPARAM, LPARAM);

/* LPARAM of DRV_CONFIGURE message */
typedef struct tagDRVCONFIGINFO
{
    DWORD   dwDCISize;
    LPCSTR  lpszDCISectionName;
    LPCSTR  lpszDCIAliasName;
} DRVCONFIGINFO;
typedef DRVCONFIGINFO NEAR* PDRVCONFIGINFO;
typedef DRVCONFIGINFO FAR* LPDRVCONFIGINFO;

typedef struct tagDRIVERINFOSTRUCT
{
    UINT    length;
    HDRVR   hDriver;
    HINSTANCE hModule;
    char    szAliasName[128];
} DRIVERINFOSTRUCT;
typedef DRIVERINFOSTRUCT FAR* LPDRIVERINFOSTRUCT;

#ifndef NOAPIPROTO  /* Prototypes */

LRESULT WINAPI DefDriverProc(DWORD dwDriverIdentifier, HDRVR driverID, UINT message, LPARAM lParam1, LPARAM lParam2);

HDRVR   WINAPI OpenDriver(LPCSTR szDriverName, LPCSTR szSectionName, LPARAM lParam2);
LRESULT WINAPI CloseDriver(HDRVR hDriver, LPARAM lParam1, LPARAM lParam2);

LRESULT WINAPI SendDriverMessage(HDRVR hDriver, UINT message, LPARAM lParam1, LPARAM lParam2);

HINSTANCE WINAPI GetDriverModuleHandle(HDRVR hDriver);

HDRVR   WINAPI GetNextDriver(HDRVR, DWORD);

BOOL    WINAPI GetDriverInfo(HDRVR, DRIVERINFOSTRUCT FAR*);

#endif              /* NOAPIPROTO */

/* Driver messages */
#define DRV_LOAD            0x0001
#define DRV_ENABLE          0x0002
#define DRV_OPEN            0x0003
#define DRV_CLOSE           0x0004
#define DRV_DISABLE         0x0005
#define DRV_FREE            0x0006
#define DRV_CONFIGURE       0x0007
#define DRV_QUERYCONFIGURE  0x0008
#define DRV_INSTALL         0x0009
#define DRV_REMOVE          0x000A
#define DRV_EXITSESSION     0x000B
#define DRV_EXITAPPLICATION 0x000C
#define DRV_POWER           0x000F

#define DRV_RESERVED        0x0800
#define DRV_USER            0x4000

/* Supported return values for DRV_CONFIGURE message */
#define DRVCNF_CANCEL       0x0000
#define DRVCNF_OK           0x0001
#define DRVCNF_RESTART      0x0002

/* Supported lParam1 of DRV_EXITAPPLICATION notification */
#define DRVEA_NORMALEXIT    0x0001
#define DRVEA_ABNORMALEXIT  0x0002

/* GetNextDriver flags */
#define GND_FIRSTINSTANCEONLY   0x00000001

#define GND_FORWARD             0x00000000
#define GND_REVERSE             0x00000002

#endif      /* NODRIVERS */

/* (WIN32) Window Management *********************************************** */

	/* ChildWindowFromPointEx() flags */
#define CWP_ALL			0x01
#define CWP_SKIPINVISIBLE	0x02
#define CWP_SKIPDISABLED	0x04
#define CWP_SKIPTRANSPARENT	0x08


#ifndef	NOAPIPROTO	/* Prototypes */

WORD	WINAPI CascadeWindows(HWND, UINT, CONST RECT *, UINT,
	CONST HWND FAR *);
HWND	WINAPI ChildWindowFromPointEx(HWND, POINT, UINT);
HWND	WINAPI FindWindowEx(HWND, HWND, LPCTSTR, LPCTSTR);
HWND	WINAPI GetForegroundWindow(VOID);
BOOL	WINAPI IsWindowUnicode(HWND);
BOOL	WINAPI SetForegroundWindow(HWND);
WORD	WINAPI TileWindows(HWND, UINT, CONST RECT *, UINT, CONST HWND FAR *);

#endif			/* NOAPIPROTO */

/* (WIN32) Window Class **************************************************** */

typedef struct tagWNDCLASSEX {
	UINT	cbSize;
	UINT	style;
	WNDPROC	lpfnWndProc;
	int	cbClsExtra;
	int	cbWndExtra;
	HANDLE	hInstance;
	HICON	hIcon;
	HCURSOR	hCursor;
	HBRUSH	hbrBackground;
	LPCTSTR	lpszMenuName;
	LPCTSTR	lpszClassName;
	HICON	hIconSm;
} WNDCLASSEX;

typedef WNDCLASSEX	*PWNDCLASSEX;
typedef WNDCLASSEX NEAR	*NPWNDCLASSEX;
typedef WNDCLASSEX FAR	*LPWNDCLASSEX;

#ifndef	NOAPIPROTO	/* Prototypes */

BOOL	WINAPI GetClassInfoEx(HINSTANCE, LPCTSTR, LPWNDCLASSEX);
ATOM	WINAPI RegisterClassEx(CONST WNDCLASSEX *);

#endif			/* NOAPIPROTO */

/* Win32 Hotkey APIs */

#define MOD_ALT        0x0001
#define MOD_CONTROL    0x0002
#define MOD_SHIFT      0x0004
#define MOD_WIN        0x0008

#define WM_HOTKEY	0x0312
#define WM_SETHOTKEY	0x0032
#define WM_GETHOTKEY	0x0033


BOOL WINAPI RegisterHotKey(HWND hwnd, int id, UINT fsModifiers, UINT vk);
BOOL WINAPI UnregisterHotKey(HWND hwnd, int id);

/* Color ******************************************************************* */

#ifndef NOCOLOR

#define COLOR_3DDKSHADOW	21
#define COLOR_3DLIGHT		22
#define COLOR_INFOTEXT		23
#define COLOR_INFOBK		24

#define COLOR_3DFACE		COLOR_BTNFACE
#define COLOR_3DSHADOW		COLOR_BTNSHADOW
#define COLOR_3DHIGHLIGHT	COLOR_BTNHIGHLIGHT
#define COLOR_3DHILIGHT		COLOR_BTNHIGHLIGHT
#define COLOR_BTNHILIGHT	COLOR_BTNHIGHLIGHT
#define COLOR_DESKTOP		COLOR_BACKGROUND

#endif

/* Window Messages ********************************************************* */

#define WM_ENTERMENULOOP	0x0211
#define WM_EXITMENULOOP		0x0212

/* Window Messages (WINVER >= 0x0400) ************************************** */

typedef struct tagMDINEXTMENU
{
    HMENU	hmenuIn;
    HMENU	hmenuNext;
    HWND	hwndNext;
} MDINEXTMENU;
typedef MDINEXTMENU		*PMDINEXTMENU;
typedef MDINEXTMENU NEAR	*NPMDINEXTMENU;
typedef MDINEXTMENU FAR		*LPMDINEXTMENU;

#define WM_NEXTMENU		0x0213
#define WM_SIZING		0x0214
#define WM_CAPTURECHANGED	0x0215
#define WM_MOVING		0x0216
#define WM_POWERBROADCAST	0x0218
#define WM_DEVICECHANGE		0x0219

#define WM_IME_SETCONTEXT	0x0281
#define WM_IME_NOTIFY		0x0282
#define WM_IME_CONTROL		0x0283
#define WM_IME_COMPOSITIONFULL	0x0284
#define WM_IME_SELECT		0x0285
#define WM_IME_CHAR		0x0286
#define WM_IME_KEYDOWN		0x0290
#define WM_IME_KEYUP		0x0291

#define WM_PRINT		0x0317
#define WM_PRINTCLIENT		0x0318

#define WM_HANDHELDFIRST	0x0358
#define WM_HANDHELDLAST		0x035F

#define WM_AFXFIRST		0x0360
#define WM_AFXLAST		0x037F

/* (WIN32) Point *********************************************************** */

#ifndef NOUSER

typedef POINT POINTL;

#endif	/* NOUSER */

/* (WIN32) Print Flags ***************************************************** */

#define PRF_CHECKVISIBLE    0x00000001L
#define PRF_NONCLIENT       0x00000002L
#define PRF_CLIENT          0x00000004L
#define PRF_ERASEBKGND      0x00000008L
#define PRF_CHILDREN        0x00000010L
#define PRF_OWNED           0x00000020L

/* (WIN32) Draw Escape ***************************************************** */

/* function prototypes */

#ifndef NOAPIPROTO

int	WINAPI
DrawEscape(HDC, int, int, LPCSTR);

#endif	/* NOAPIPROTO */

/* (WIN32) User Interface Drawing Functions ******************************** */

/* DrawEdge() edge */

#define BDR_RAISEDOUTER		0x0001
#define BDR_SUNKENOUTER		0x0002
#define BDR_RAISEDINNER		0x0004
#define BDR_SUNKENINNER		0x0008

#define BDR_OUTER		(BDR_RAISEDOUTER | BDR_SUNKENOUTER)
#define BDR_INNER		(BDR_RAISEDINNER | BDR_SUNKENINNER)

#define EDGE_BUMP		(BDR_RAISEDOUTER | BDR_SUNKENINNER)
#define EDGE_ETCHED		(BDR_SUNKENOUTER | BDR_RAISEDINNER)
#define EDGE_RAISED		(BDR_RAISEDOUTER | BDR_RAISEDINNER)
#define EDGE_SUNKEN		(BDR_SUNKENOUTER | BDR_SUNKENINNER)

/* DrawEdge() border */

#define BF_ADJUST			0x0001
#define BF_BOTTOM			0x0002
#define BF_BOTTOMLEFT			0x0004
#define BF_BOTTOMRIGHT			0x0008
#define BF_DIAGONAL			0x0010
#define BF_DIAGONAL_ENDBOTTOMLEFT	0x0020
#define BF_DIAGONAL_ENDBOTTOMRIGHT	0x0040
#define BF_DIAGONAL_ENDTOPLEFT		0x0081
#define BF_DIAGONAL_ENDTOPRIGHT		0x0101
#define BF_FLAT				0x0201
#define BF_LEFT				0x0401
#define BF_MIDDLE			0x0801
#define BF_MONO				0x1001
#define BF_RECT				0x2001
#define BF_RIGHT			0x4001
#define BF_SOFT				0x8001
#define BF_TOP				0x1101
#define BF_TOPLEFT			0x2201
#define BF_TOPRIGHT			0x3301

/* DrawFrameControl() type */

#define DFC_BUTTON		0x0001
#define DFC_CAPTION		0x0002
#define DFC_MENU		0x0004
#define DFC_SCROLL		0x0008

/* DrawFrameControl() state */

#define DFCS_BUTTON3STATE	0x0001
#define DFCS_BUTTONCHECK	0x0002
#define DFCS_BUTTONPUSH		0x0004
#define DFCS_BUTTONRADIO	0x0008
#define DFCS_BUTTONRADIOIMAGE	0x0010
#define DFCS_BUTTONRADIOMASK	0x0020

#define DFCS_CAPTIONCLOSE	0x0001
#define DFCS_CAPTIONHELP	0x0002
#define DFCS_CAPTIONMAX		0x0004
#define DFCS_CAPTIONMIN		0x0008
#define DFCS_CAPTIONRESTORE	0x0010

#define DFCS_MENUARROW		0x0001
#define DFCS_MENUBULLET		0x0002
#define DFCS_MENUCHECK		0x0004

#define DFCS_SCROLLCOMBOBOX	0x0001
#define DFCS_SCROLLDOWN		0x0002
#define DFCS_SCROLLLEFT		0x0004
#define DFCS_SCROLLRIGHT	0x0008
#define DFCS_SCROLLSIZEGRIP	0x0010
#define DFCS_SCROLLUP		0x0020

#define DFCS_ADJUSTRECT		0x0100

#define DFCS_CHECKED		0x1000
#define DFCS_FLAT		0x2000
#define DFCS_INACTIVE		0x4000
#define DFCS_MONO		0x8000
#define DFCS_PUSHED		0x8200

/* DrawState() type/state */

#define DST_BITMAP		0x0001
#define DST_COMPLEX		0x0002
#define DST_ICON		0x0004
#define DST_PREFIXTEXT		0x0008
#define DST_TEXT		0x0010

#define DSS_NORMAL		0x0001
#define DSS_UNION		0x0002
#define DSS_DISABLED		0x0004
#define DSS_MONO		0x0008

/* function prototypes */

#ifndef NOAPIPROTO

typedef BOOL (CALLBACK *DRAWSTATEPROC)(HDC, LPARAM, WPARAM, int, int);

BOOL	WINAPI
DrawEdge(HDC hDC, LPRECT lpRect, UINT uEdge, UINT uBorder);

BOOL	WINAPI
DrawFrameControl(HDC hDC, LPRECT lpRect, UINT uType, UINT uState);

BOOL	WINAPI
DrawState(HDC hDC, HBRUSH hBrush, DRAWSTATEPROC lpDrawStateProc,
	LPARAM lParam, WPARAM wParam,
	int x, int y, int cx, int cy,
	UINT uFlags);

#endif	/* NOAPIPROTO */

/* (WIN32) Help Context Id ************************************************* */

/* function prototypes */

#ifndef NOAPIPROTO

DWORD	WINAPI
GetMenuContextHelpId(HMENU);

BOOL	WINAPI
SetMenuContextHelpId(HMENU, DWORD);

DWORD	WINAPI
GetWindowContextHelpId(HWND);

BOOL	WINAPI
SetWindowContextHelpId(HWND, DWORD);

#endif	/* NOAPIPROTO */

/* (WIN32) Character Conversion ******************************************** */

/* function prototypes */

#ifndef NOAPIPROTO

LPTSTR	WINAPI
CharLower(LPTSTR);

LPTSTR	WINAPI
CharUpper(LPTSTR);

DWORD	WINAPI
CharLowerBuff(LPTSTR, DWORD);

DWORD	WINAPI
CharUpperBuff(LPTSTR, DWORD);

LPTSTR	WINAPI
CharNext(LPCTSTR);

LPTSTR	WINAPI
CharPrev(LPCTSTR, LPCTSTR);

#endif	/* NOAPIPROTO */

/* (WIN32) Menu ************************************************************ */

/* function prototypes */

#ifndef NOAPIPROTO

BOOL	WINAPI
CheckMenuRadioItem(HMENU hMenu,
	UINT uFirstMenuItem, UINT uLastMenuItem,
	UINT uCheckMenuItem, UINT uMenuFlags);

#endif	/* NOAPIPROTO */

/* (WIN32) Message Synchronization ***************************************** */

/* function prototypes */

#ifndef NOAPIPROTO

DWORD	WINAPI
MsgWaitForMultipleObjects(DWORD dwCount,
	LPHANDLE lpHandles,
	BOOL bWaitAll,
	DWORD dwMilliseconds,
	DWORD dwWakeMask);

#endif	/* NOAPIPROTO */

/* (WIN32) ***************************************************************** */

/* function prototypes */

#ifndef NOAPIPROTO

#endif	/* NOAPIPROTO */

#define LANG_NEUTRAL                     0x00

#define LANG_AFRIKAANS                   0x36
#define LANG_ALBANIAN                    0x1c
#define LANG_BASQUE                      0x2d
#define LANG_BELARUSIAN                  0x23
#define LANG_BULGARIAN                   0x02
#define LANG_CATALAN                     0x03
#define LANG_CHINESE                     0x04
#define LANG_CROATIAN                    0x1a
#define LANG_CZECH                       0x05
#define LANG_DANISH                      0x06
#define LANG_DUTCH                       0x13
#define LANG_ENGLISH                     0x09
#define LANG_ESTONIAN                    0x25
#define LANG_FAEROESE                    0x38
#define LANG_FINNISH                     0x0b
#define LANG_FRENCH                      0x0c
#define LANG_GERMAN                      0x07
#define LANG_GREEK                       0x08
#define LANG_HUNGARIAN                   0x0e
#define LANG_ICELANDIC                   0x0f
#define LANG_INDONESIAN                  0x21
#define LANG_ITALIAN                     0x10
#define LANG_JAPANESE                    0x11
#define LANG_KOREAN                      0x12
#define LANG_LATVIAN                     0x26
#define LANG_LITHUANIAN                  0x27
#define LANG_NORWEGIAN                   0x14
#define LANG_POLISH                      0x15
#define LANG_PORTUGUESE                  0x16
#define LANG_ROMANIAN                    0x18
#define LANG_RUSSIAN                     0x19
#define LANG_SERBIAN                     0x1a
#define LANG_SLOVAK                      0x1b
#define LANG_SLOVENIAN                   0x24
#define LANG_SPANISH                     0x0a
#define LANG_SWEDISH                     0x1d
#define LANG_THAI                        0x1e
#define LANG_TURKISH                     0x1f
#define LANG_UKRAINIAN                   0x22
#define LANG_VIETNAMESE                  0x2a

#define SUBLANG_NEUTRAL                  0x00    /* language neutral */
#define SUBLANG_DEFAULT                  0x01    /* user default */
#define SUBLANG_SYS_DEFAULT              0x02    /* system default */

#define SUBLANG_CHINESE_TRADITIONAL      0x01    /* Chinese (Taiwan) */
#define SUBLANG_CHINESE_SIMPLIFIED       0x02    /* Chinese (PR China) */
#define SUBLANG_CHINESE_HONGKONG         0x03    /* Chinese (Hong Kong) */
#define SUBLANG_CHINESE_SINGAPORE        0x04    /* Chinese (Singapore) */
#define SUBLANG_DUTCH                    0x01    /* Dutch */
#define SUBLANG_DUTCH_BELGIAN            0x02    /* Dutch (Belgian) */
#define SUBLANG_ENGLISH_US               0x01    /* English (USA) */
#define SUBLANG_ENGLISH_UK               0x02    /* English (UK) */
#define SUBLANG_ENGLISH_AUS              0x03    /* English (Australian) */
#define SUBLANG_ENGLISH_CAN              0x04    /* English (Canadian) */
#define SUBLANG_ENGLISH_NZ               0x05    /* English (New Zealand) */
#define SUBLANG_ENGLISH_EIRE             0x06    /* English (Irish) */
#define SUBLANG_ENGLISH_SOUTH_AFRICA     0x07    /* English (South Africa) */
#define SUBLANG_ENGLISH_JAMAICA          0x08    /* English (Jamaica) */
#define SUBLANG_ENGLISH_CARIBBEAN        0x09    /* English (Caribbean) */
#define SUBLANG_ENGLISH_BELIZE           0x0a    /* English (Belize) */
#define SUBLANG_ENGLISH_TRINIDAD         0x0b    /* English (Trinidad) */
#define SUBLANG_FRENCH                   0x01    /* French */
#define SUBLANG_FRENCH_BELGIAN           0x02    /* French (Belgian) */
#define SUBLANG_FRENCH_CANADIAN          0x03    /* French (Canadian) */
#define SUBLANG_FRENCH_SWISS             0x04    /* French (Swiss) */
#define SUBLANG_FRENCH_LUXEMBOURG        0x05    /* French (Luxembourg) */
#define SUBLANG_GERMAN                   0x01    /* German */
#define SUBLANG_GERMAN_SWISS             0x02    /* German (Swiss) */
#define SUBLANG_GERMAN_AUSTRIAN          0x03    /* German (Austrian) */
#define SUBLANG_GERMAN_LUXEMBOURG        0x04    /* German (Luxembourg) */
#define SUBLANG_GERMAN_LIECHTENSTEIN     0x05    /* German (Liechtenstein) */
#define SUBLANG_ITALIAN                  0x01    /* Italian */
#define SUBLANG_ITALIAN_SWISS            0x02    /* Italian (Swiss) */
#define SUBLANG_NORWEGIAN_BOKMAL         0x01    /* Norwegian (Bokmal) */
#define SUBLANG_NORWEGIAN_NYNORSK        0x02    /* Norwegian (Nynorsk) */
#define SUBLANG_PORTUGUESE               0x02    /* Portuguese */
#define SUBLANG_PORTUGUESE_BRAZILIAN     0x01    /* Portuguese (Brazilian) */
#define SUBLANG_SERBIAN_LATIN            0x02    /* Serbian (Latin) */
#define SUBLANG_SERBIAN_CYRILLIC         0x03    /* Serbian (Cyrillic) */
#define SUBLANG_SPANISH                  0x01    /* Spanish (Castilian) */
#define SUBLANG_SPANISH_MEXICAN          0x02    /* Spanish (Mexican) */
#define SUBLANG_SPANISH_MODERN           0x03    /* Spanish (Modern) */
#define SUBLANG_SPANISH_GUATEMALA        0x04    /* Spanish (Guatemala) */
#define SUBLANG_SPANISH_COSTA_RICA       0x05    /* Spanish (Costa Rica) */
#define SUBLANG_SPANISH_PANAMA           0x06    /* Spanish (Panama) */
#define SUBLANG_SPANISH_DOMINICAN_REPUBLIC 0x07  /* Spanish (Dominican Republic) */
#define SUBLANG_SPANISH_VENEZUELA        0x08    /* Spanish (Venezuela) */
#define SUBLANG_SPANISH_COLOMBIA         0x09    /* Spanish (Colombia) */
#define SUBLANG_SPANISH_PERU             0x0a    /* Spanish (Peru) */
#define SUBLANG_SPANISH_ARGENTINA        0x0b    /* Spanish (Argentina) */
#define SUBLANG_SPANISH_ECUADOR          0x0c    /* Spanish (Ecuador) */
#define SUBLANG_SPANISH_CHILE            0x0d    /* Spanish (Chile) */
#define SUBLANG_SPANISH_URUGUAY          0x0e    /* Spanish (Uruguay) */
#define SUBLANG_SPANISH_PARAGUAY         0x0f    /* Spanish (Paraguay) */
#define SUBLANG_SPANISH_BOLIVIA          0x10    /* Spanish (Bolivia) */
#define SUBLANG_SPANISH_EL_SALVADOR      0x11    /* Spanish (El Salvador) */
#define SUBLANG_SPANISH_HONDURAS         0x12    /* Spanish (Honduras) */
#define SUBLANG_SPANISH_NICARAGUA        0x13    /* Spanish (Nicaragua) */
#define SUBLANG_SPANISH_PUERTO_RICO      0x14    /* Spanish (Puerto Rico) */
#define SUBLANG_SWEDISH                  0x01    /* Swedish */
#define SUBLANG_SWEDISH_FINLAND          0x02    /* Swedish (Finland) */

#define SORT_DEFAULT                     0x0     /* sorting default */

#define SORT_JAPANESE_XJIS               0x0     /* Japanese XJIS order */
#define SORT_JAPANESE_UNICODE            0x1     /* Japanese Unicode order */

#define SORT_CHINESE_BIG5                0x0     /* Chinese BIG5 order */
#define SORT_CHINESE_PRCP                0x0     /* PRC Chinese Phonetic order */
#define SORT_CHINESE_UNICODE             0x1     /* Chinese Unicode order */
#define SORT_CHINESE_PRC                 0x2     /* PRC Chinese Stroke */

#define SORT_KOREAN_KSC                  0x0     /* Korean KSC order */
#define SORT_KOREAN_UNICODE              0x1     /* Korean Unicode order */

#define SORT_GERMAN_PHONE_BOOK           0x1     /* German Phone Book order */

/*
//  A language ID is a 16 bit value which is the combination of a
//  primary language ID and a secondary language ID.  The bits are
//  allocated as follows:
//
//       +-----------------------+-------------------------+
//       |     Sublanguage ID    |   Primary Language ID   |
//       +-----------------------+-------------------------+
//        15                   10 9                       0   bit
//
//
//  Language ID creation/extraction macros:
//
//    MAKELANGID    - construct language id from a primary language id and
//                    a sublanguage id.
//    PRIMARYLANGID - extract primary language id from a language id.
//    SUBLANGID     - extract sublanguage id from a language id.
*/

#define MAKELANGID(p, s)       ((((WORD  )(s)) << 10) | (WORD  )(p))
#define PRIMARYLANGID(lgid)    ((WORD  )(lgid) & 0x3ff)
#define SUBLANGID(lgid)        ((WORD  )(lgid) >> 10)


/*
//  A locale ID is a 32 bit value which is the combination of a
//  language ID, a sort ID, and a reserved area.  The bits are
//  allocated as follows:
//
//       +-------------+---------+-------------------------+
//       |   Reserved  | Sort ID |      Language ID        |
//       +-------------+---------+-------------------------+
//        31         20 19     16 15                      0   bit
//
//
//  Locale ID creation/extraction macros:
//
//    MAKELCID       - construct locale id from a language id and a sort id.
//    LANGIDFROMLCID - extract language id from a locale id.
//    SORTIDFROMLCID - extract sort id from a locale id.
*/

#define NLS_VALID_LOCALE_MASK  0x000fffff

#define MAKELCID(lgid, srtid)  ((DWORD)((((DWORD)((WORD  )(srtid))) << 16) |  \
                                         ((DWORD)((WORD  )(lgid)))))
#define LANGIDFROMLCID(lcid)   ((WORD  )(lcid))
#define SORTIDFROMLCID(lcid)   ((WORD  )((((DWORD)(lcid)) & NLS_VALID_LOCALE_MASK) >> 16))


/*
//  Default System and User IDs for language and locale.
*/

#define LANG_SYSTEM_DEFAULT    (MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT))
#define LANG_USER_DEFAULT      (MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT))

#define LOCALE_SYSTEM_DEFAULT  (MAKELCID(LANG_SYSTEM_DEFAULT, SORT_DEFAULT))
#define LOCALE_USER_DEFAULT    (MAKELCID(LANG_USER_DEFAULT, SORT_DEFAULT))

#define LOCALE_NEUTRAL \
          (MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT))

/*added by chris for chicoapp.c*/
#define MIIM_STATE       0x00000001
#define MIIM_ID          0x00000002
#define MIIM_SUBMENU     0x00000004
#define MIIM_CHECKMARKS  0x00000008
#define MIIM_TYPE        0x00000010
#define MIIM_DATA        0x00000020

typedef struct tagMENUITEMINFOA
{
    UINT    cbSize;
    UINT    fMask;
    UINT    fType;              
    UINT    fState;         
    UINT    wID;            
    HMENU   hSubMenu;       
    HBITMAP hbmpChecked;    
    HBITMAP hbmpUnchecked;  
    DWORD   dwItemData;     
    LPSTR   dwTypeData;     
    UINT    cch;            
}   MENUITEMINFOA, FAR *LPMENUITEMINFOA;
typedef struct tagMENUITEMINFOW
{
    UINT    cbSize;
    UINT    fMask;
    UINT    fType;          
    UINT    fState;         
    UINT    wID;            
    HMENU   hSubMenu;       
    HBITMAP hbmpChecked;    
    HBITMAP hbmpUnchecked;  
    DWORD   dwItemData;     
    LPWSTR  dwTypeData;     
    UINT    cch;            
}   MENUITEMINFOW, FAR *LPMENUITEMINFOW;
#ifdef UNICODE
typedef MENUITEMINFOW MENUITEMINFO;
typedef LPMENUITEMINFOW LPMENUITEMINFO;
#else
typedef MENUITEMINFOA MENUITEMINFO;
typedef LPMENUITEMINFOA LPMENUITEMINFO;
#endif /* UNICODE */
typedef MENUITEMINFOA CONST FAR *LPCMENUITEMINFOA;
typedef MENUITEMINFOW CONST FAR *LPCMENUITEMINFOW;
#ifdef UNICODE
typedef LPCMENUITEMINFOW LPCMENUITEMINFO;
#else
typedef LPCMENUITEMINFOA LPCMENUITEMINFO;
#endif /* UNICODE */
#define MFS_GRAYED          0x00000003L
#define MFS_DISABLED        MFS_GRAYED 
#define MFS_CHECKED         MF_CHECKED 
#define MFS_HILITE          MF_HILITE
#define MFS_ENABLED         MF_ENABLED 
#define MFS_UNCHECKED       MF_UNCHECKED
#define MFS_UNHILITE        MF_UNHILITE
#define MFS_DEFAULT         MF_DEFAULT 


#endif      /* Win_User__h */


