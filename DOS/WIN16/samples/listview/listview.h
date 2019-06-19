/****************************************************************************
*
*    ListView.h
*    @(#)listview.h	1.1: /users/sccs/src/samples/listview/s.listview.h 12/18/96 16:41:31
*
*    PURPOSE: Demonstrates the use of the new list view control in Chicago
*
****************************************************************************/


#define ID_LISTVIEW     1000

#define NUM_ITEMS       9
#define NUM_COLUMNS     5
#define MAX_ITEMLEN     64
#define MAX_ADDRESS     64
#define MAX_CITY        16
#define MAX_PRICE       12
#define BITMAP_WIDTH    16
#define BITMAP_HEIGHT   16
#define LG_BITMAP_WIDTH	32
#define LG_BITMAP_HEIGHT 32





#define IDM_LARGEICON   100
#define IDM_SMALLICON   101
#define IDM_LISTVIEW    102
#define IDM_REPORTVIEW  103
#define IDM_EXIT        104
#define IDM_GETORIGIN   40001
#define IDM_GETSELCOUNT 40002
#define IDM_GETCOUNTPERPAGE             40003
#define IDM_GETITEMRECT                 40004
#define IDM_ENSVISIBLE                  40005
#define IDM_ALIGNTOP                    40006
#define IDM_ALIGNLEFT                   40007

#define IDM_ABOUT       300


#define APP_ICON        400
#define REDMOND         401
#define BELLEVUE        402
#define SEATTLE         403


#define IDS_ADDRESS     1
#define IDS_CITY        2
#define IDS_PRICE       3
#define IDS_BEDS        4
#define IDS_BATHS       5




long APIENTRY MainWndProc(HWND, UINT, UINT, LONG);
BOOL APIENTRY About(HWND, UINT, UINT, LONG);


BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
HWND CreateListView(HWND);
LRESULT NotifyHandler(HWND, UINT, WPARAM, LPARAM);
int CALLBACK ListViewCompareProc(LPARAM, LPARAM, LPARAM);

extern int WINAPI LView_Initialize 
(
	HINSTANCE		hinst
);

extern void WINAPI LView_Terminate 
(
	HINSTANCE			hInstance
);

extern int WINAPI Headr_Initialize 
(
	HINSTANCE		hinst
);

extern void WINAPI Headr_Terminate 
(
	HINSTANCE			hInstance
);


typedef struct tagHOUSEINFO
{
    char szAddress[MAX_ADDRESS];
    char szCity[MAX_CITY];
    int iPrice;
    int iBeds;
    int iBaths;
} HOUSEINFO;



