/*
 *      MsgDemo.h
 *
 *      @(#)msgdemo.h	1.1 5/3/96 16:39:29 /users/sccs/src/samples/msgdemo/s.msgdemo.h
 *
 *      Simple Windows Message Demonstration Application
 */

#include <windows.h>
#include "msgcode.h"

/* function declarations */

void FAR PASCAL ShellAbout(HWND, LPCSTR, LPCSTR, HICON);
MSGCODE *GetTwinMsgCode(int);
void report(HWND ,int ,WPARAM , LPARAM );
BOOL MsgTypeOk(int);
void MsgPaint(HWND);
int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
long FAR PASCAL __export MsgWndProc(HWND, UINT, WPARAM, LPARAM);

/* Resource-IDs */
#define IDM_ABOUT       102
#define IDM_EXIT        103
#define IDM_KEY         104
#define IDM_MOUSE       105
#define IDM_WINDOW      106
#define IDM_ALL         107

#define DLL_APP_TITLE   200

#define IDI_ICON1 100

/* Display 20 Messages */
#define MSG_LINES       21

/* Type of messages to be displayed */
#define MSG_KEY         0x01
#define MSG_MOUSE       0x02
#define MSG_WINDOW      0x04
#define MSG_ALL         0x08

/* Struct for displayed information */
typedef struct tagMSGINFO {
    int id;
    WPARAM wParam;
    LPARAM lParam;
    UINT   count;
} MSGINFO; 
 
