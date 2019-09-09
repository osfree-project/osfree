/*
 *      BinDemo.h
 *
 *      @(#)bindemo.h	1.1 5/15/96 16:36:56 /users/sccs/src/samples/bn2demo/s.bindemo.h
 *
 *      Dynamic Linking Sample Application
 */

/* declaration of WINDOWS callback routines */
long FAR PASCAL BinDemoWndProc(HWND, unsigned, WORD, LONG);
void FAR PASCAL ShellAbout(HWND, LPCSTR, LPCSTR, HICON);

/* other functions */
void SetCardMarks(HWND, UINT);
void ClearCardMarks(HWND, UINT, UINT);

/* file menu items */
#define     IDM_EXIT        100
#define     IDM_ABOUT       101
/* cards menu items */
#define     IDM_CLUBS       102
#define     IDM_DIAMONDS    103
#define     IDM_HEARTS      104
#define     IDM_SPADES      105
#define     IDM_BACKSIDES   106
/* card size */
#define     IDM_SMALL       110
#define     IDM_NORMAL      111
#define     IDM_LARGE       112

/* stuff dealing with cards */
#define     ID_FIRSTCARDID  IDM_CLUBS
#define     ID_LASTCARDID   IDM_BACKSIDES
#define     ID_FIRSTSIZEID  IDM_SMALL
#define     ID_LASTSIZEID   IDM_LARGE

#define RGBBKND RGB(255,255,255)

