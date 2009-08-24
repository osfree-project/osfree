










#ifndef RC_INVOKED

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#define OEMRESOURCE
#include "winext.h"






#define PALETTE_SIZE 16
typedef enum
{
	BLACK, DKGREY, GREY, WHITE,
	DKRED, RED, DKGREEN, GREEN, DKBLUE, BLUE,
	DKYELLOW, YELLOW, DKCYAN, CYAN, DKMAGENTA, MAGENTA
} COLORS;


#define DEGREE_SIZE 256
#define DEGREE_MASK 255
#define DEGREE_MAX 0x4000


#define MAX_PTS 8
#define MAX_OBJS 100
#define MAX_COORD 0x2000
#define CLIP_COORD (MAX_COORD+300)


#define DRAW_TIMER 1
#define DRAW_DELAY 50
#define RESTART_TIMER 2
#define RESTART_DELAY 5000


typedef enum { RESTART_GAME, RESTART_LEVEL, RESTART_NEXTLEVEL } RESTART_MODE;


#define LETTER_MAX 256


#define EXTRA_LIFE 100000


typedef struct tagNODE
{
	struct tagNODE  *npNext, *npPrev;
} NODE;
pointerdef( NODE );


typedef struct
{
	NPNODE          npHead, npTail;
} LIST;
pointerdef( LIST );


typedef struct
{
	NODE            Link;               
	POINT           Pos;                
	POINT           Vel;                
	INT             nMass;              
	INT             nDir;               
	INT             nSpin;              
	INT             nCount;             
	INT             nDelay;             
	BYTE            byColor;            
	BYTE            byPts;              
	POINT           Pts[MAX_PTS];       
	POINT           Old[MAX_PTS];       
} OBJ;
pointerdef( OBJ );






#define AddHeadObj(l,o) AddHead((l),((NPNODE)o))
#define RemHeadObj(l) ((NPOBJ)RemHead(l))
#define RemoveObj(l,o) Remove((l),((NPNODE)o))
#define HeadObj(l) ((NPOBJ)((l)->npHead))
#define NextObj(o) ((NPOBJ)((o)->Link.npNext))


#define IsKeyDown(x) (GetAsyncKeyState(x)<0)


#define INTRES(x) MAKEINTRESOURCE(x)


#define DIM(x) (sizeof(x)/sizeof((x)[0]))


#define MULDEG(x,y) ((INT)(((LONG)(x)*(y))/DEGREE_MAX))


#define DEG(x) ((WORD)(x)&DEGREE_MASK)


#define ACCEL(o,d,s) \
(((o)->Vel.x += MULDEG((s),nCos[DEG(d)])), \
((o)->Vel.y += MULDEG((s),nSin[DEG(d)])))


#define PTINRECT(r,p) \
(((r)->left <= (p).x) && ((r)->right > (p).x) && \
((r)->top <= (p).y) && ((r)->bottom > (p).y))


#define INTRECT(r1,r2) \
(((r1)->right >= (r2)->left) && \
((r1)->left < (r2)->right) && \
((r1)->bottom >= (r2)->top) && \
((r1)->top < (r2)->bottom))


#define MKRECT(r,p,s) \
(((r)->left = ((p).x-(s))), ((r)->right = ((p).x+(s))), \
((r)->top = ((p).y-(s))), ((r)->bottom = ((p).y+(s))))






INT NEAR PASCAL arand( INT x );
VOID NEAR PASCAL AddHead( NPLIST npList, NPNODE npNode );
NPNODE NEAR PASCAL RemHead( NPLIST npList );
VOID NEAR PASCAL Remove( NPLIST npList, NPNODE npNode );
VOID NEAR PASCAL DrawObject( HDC hDC, NPOBJ npObj );
VOID NEAR PASCAL SetRestart( RESTART_MODE bGameOver );
VOID NEAR PASCAL AddExtraLife( VOID );
VOID NEAR PASCAL Hit( HDC hDC, NPOBJ npObj );
VOID NEAR PASCAL Explode( HDC hDC, NPOBJ npObj );
BOOL NEAR PASCAL HitPlayer( HDC hDC, NPOBJ npObj );
NPOBJ FAR PASCAL CreateLetter( CHAR cLetter, INT nSize );
VOID NEAR PASCAL DrawLetters( HDC hDC );
VOID NEAR PASCAL DrawHunterShots( HDC hDC );
VOID NEAR PASCAL FireHunterShot( NPOBJ npHunt );
VOID NEAR PASCAL CreateHunter( VOID );
VOID NEAR PASCAL DrawHunters( HDC hDC );
VOID NEAR PASCAL CreateSpinner( VOID );
VOID NEAR PASCAL DrawSpinners( HDC hDC );
VOID NEAR PASCAL CreateRoid( POINT Pos, POINT Vel, INT nSides, BYTE byColor, INT nDir, INT nSpeed, INT nSpin );
VOID NEAR PASCAL BreakRoid( HDC hDC, NPOBJ npRoid, NPOBJ npShot );
VOID NEAR PASCAL DrawRoids( HDC hDC );
VOID NEAR PASCAL DrawShots( HDC hDC );
VOID NEAR PASCAL DrawFlames( HDC hDC );
VOID NEAR PASCAL FireShot( VOID );
VOID NEAR PASCAL AccelPlayer( INT nDir, INT nAccel );
VOID NEAR PASCAL DrawPlayer( HDC hDC );
VOID NEAR PASCAL DrawObjects( HWND hWnd );
VOID NEAR PASCAL CheckScore( HWND hWnd );
VOID NEAR PASCAL HitList( HDC hDC, NPLIST npList );
VOID NEAR PASCAL ExplodeBadguys( HDC hDC, NPLIST npList );
VOID NEAR PASCAL NewGame( HWND hWnd );
VOID NEAR PASCAL RestartHyperoid( VOID );
VOID NEAR PASCAL Panic( BOOL bPanic );
VOID NEAR PASCAL PaintHyperoid( HWND hWnd );
VOID NEAR PASCAL DisableHyperoidInput( HWND hWnd, BOOL bCapture );
LONG FAR PASCAL EXPORT HyperoidWndProc( HWND hWnd, unsigned message, WORD wParam, LONG lParam );
BOOL NEAR PASCAL InitHyperoid( VOID );
VOID NEAR PASCAL ExitHyperoid( VOID );
INT FAR PASCAL WinMain( HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdLine, INT nCmdShow );


VOID FAR PASCAL PrintLetters( NPSTR npszText, POINT Pos, POINT Vel, BYTE byColor, INT nSize );
VOID FAR PASCAL SpinLetters( NPSTR npszText, POINT Pos, POINT Vel, BYTE byColor, INT nSize );
HPALETTE FAR PASCAL CreateHyperoidPalette( VOID );
BOOL FAR PASCAL CreateHyperoidClass( VOID );
VOID NEAR PASCAL SetHyperoidMenu( HWND hWnd, INT nFirstID, INT nLastID );
HWND FAR PASCAL CreateHyperoidWindow( LPSTR lpszCmd, INT nCmdShow );
VOID FAR PASCAL SaveHyperoidWindowPos( HWND hWnd );
VOID FAR PASCAL GetHyperoidIni( VOID );
VOID FAR PASCAL HyperoidHelp( HWND hWnd );
BOOL FAR PASCAL EXPORT HyperoidAboutDlg( HWND hDlg, WORD mess, WORD wParam, LONG lParam );
VOID FAR PASCAL AboutHyperoid( HWND hWnd );

#endif 






#define IDI_HYPEROID    10
#define IDI_PANIC       20


#define IDB_blank       50
#define IDB_bomb        51
#define IDB_level       52
#define IDB_life        53
#define IDB_num0        54
#define IDB_num1        55
#define IDB_num2        56
#define IDB_num3        57
#define IDB_num4        58
#define IDB_num5        59
#define IDB_num6        60
#define IDB_num7        61
#define IDB_num8        62
#define IDB_num9        63
#define IDB_plus        64
#define IDB_score       65
#define IDB_shield      66

#define IDB_MAX         17
#define CX_BITMAP       16
#define CY_BITMAP       16


#define IDS_NAME        100


#define IDM_NEW         200
#define IDM_ABOUT       201


#define IDD_ABOUT       500
#define IDD_A_HELP      501
#define IDD_A_HISCORE   502

