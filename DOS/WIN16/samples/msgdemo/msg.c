/*
 *      MsgDemo.c
 *
 *      @(#)msgdemo.c	1.1 5/3/96 16:39:22 /users/sccs/src/samples/msgdemo/s.msgdemo.c
 *
 *      Simple Windows Message Demonstration Application
 *
 *      Demonstrates the following:
 *
 *      - use of resource and module definition compilers.
 *      - registering classes, and creating windows.
 *      - standard message loop and message dispatch logic.
 *      - use of resources, menues, stringtables and accelerators.
 *      - calling the ShellAbout library call.
 */

#include <stdio.h>
#include <string.h>
#include "windows.h"
#include "msgdemo.h"

HANDLE  hInstance;
HACCEL  hAccelTable;
HBRUSH	hBrush;

char    Program[] = "MsgDemo";
char    Title[256] = "";

char    MsgBuffer [80];         /* Stringbuffer for displaying Messages */
MSGINFO Messages[MSG_LINES];    /* Array cont. the messages to be displayed */
UINT    MsgType;                /* Types of messages displayed */
int     MsgIndFirst;            /* Index of first displayed msg in Array */
int     MsgIndLast;             /* Index of last displayed msg in Array */

long FAR PASCAL _export 
MsgWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HMENU       hMenu;
    HICON       hIcon;
    static HINSTANCE hLibrary;

    report(hWnd,message,wParam,lParam);

    switch (message) {
    
    case WM_COMMAND:
      switch (wParam) {
        case IDM_ABOUT:
           hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
           ShellAbout(hWnd, Program, Title, hIcon);
           DestroyIcon(hIcon);
           break;

        case IDM_EXIT:
           DestroyWindow(hWnd);
           break;

        case IDM_KEY:   /* Display Keyboard-msgs from now on */
           hMenu = GetMenu(hWnd);
           if(GetMenuState(hMenu, IDM_KEY, MF_BYCOMMAND) == MF_UNCHECKED) {
               CheckMenuItem(hMenu, IDM_KEY, MF_BYCOMMAND | MF_CHECKED);
               MsgType |= MSG_KEY;
           }
           else {
               CheckMenuItem(hMenu, IDM_KEY, MF_BYCOMMAND | MF_UNCHECKED);
               MsgType &= ~MSG_KEY;
           }
           MsgIndFirst = 0; 
           MsgIndLast = 0;
           InvalidateRect(hWnd, NULL, TRUE);        
           break;

        case IDM_MOUSE:         /* Display Mouse-Msgs from now on */
           hMenu = GetMenu(hWnd);
           if(GetMenuState(hMenu, IDM_MOUSE, MF_BYCOMMAND) == MF_UNCHECKED) {
               CheckMenuItem(hMenu, IDM_MOUSE, MF_BYCOMMAND | MF_CHECKED);
               MsgType |= MSG_MOUSE;
           }
           else {
               CheckMenuItem(hMenu, IDM_MOUSE, MF_BYCOMMAND | MF_UNCHECKED);
               MsgType &= ~MSG_MOUSE;
           }
           MsgIndFirst = 0;
           MsgIndLast = 0;
           InvalidateRect(hWnd, NULL, TRUE);
           break;
            
        case IDM_WINDOW:        /* Display Window-Msgs from now on */
           hMenu = GetMenu(hWnd);
           if(GetMenuState(hMenu, IDM_WINDOW, MF_BYCOMMAND) == MF_UNCHECKED) {
               CheckMenuItem(hMenu, IDM_WINDOW,MF_BYCOMMAND | MF_CHECKED);
               MsgType |= MSG_WINDOW;
           }
           else {
               CheckMenuItem(hMenu, IDM_WINDOW,MF_BYCOMMAND | MF_UNCHECKED);
               MsgType &= ~MSG_WINDOW;
           }
           MsgIndFirst = 0;
           MsgIndLast = 0;
           InvalidateRect(hWnd, NULL, TRUE);
           break;

        case IDM_ALL:   /* Display all messages from now on */
           hMenu = GetMenu(hWnd);
           if(GetMenuState(hMenu, IDM_ALL, MF_BYCOMMAND) == MF_UNCHECKED) {
                     CheckMenuItem(hMenu, IDM_ALL, MF_BYCOMMAND | MF_CHECKED);
                     MsgType = MSG_ALL;
                     ModifyMenu(hMenu, 
                                IDM_KEY,
                                MF_BYCOMMAND | MF_STRING,
                                IDM_KEY,
                                "No &Key");
                     ModifyMenu(hMenu,
                                IDM_MOUSE,
                                MF_BYCOMMAND | MF_STRING,
                                IDM_MOUSE,
                                "No &Mouse");
                     ModifyMenu(hMenu,
                                IDM_WINDOW,
                                MF_BYCOMMAND | MF_STRING,
                                IDM_WINDOW,
                                "No &Window");
           }
           else {
                     CheckMenuItem(hMenu, IDM_ALL, MF_BYCOMMAND | MF_UNCHECKED);
                     MsgType = 0;       
                     ModifyMenu(hMenu,
                                IDM_KEY,
                                MF_BYCOMMAND | MF_STRING,
                                IDM_KEY,
                                "&Key");
                     ModifyMenu(hMenu,
                                IDM_MOUSE,
                                MF_BYCOMMAND | MF_STRING,
                                IDM_MOUSE,
                                "&Mouse");
                     ModifyMenu(hMenu,
                                IDM_WINDOW,
                                MF_BYCOMMAND | MF_STRING,
                                IDM_WINDOW,
                                "&Window");
           }
           MsgIndFirst = 0; 
           MsgIndLast = 0;    
           InvalidateRect(hWnd, NULL, TRUE);
           break;               
       }
             break;

    case WM_SIZE:
           InvalidateRect(hWnd,NULL, TRUE);
           break;

    case WM_PAINT:              /* Paint Client-Area */
           MsgPaint(hWnd);
           break;

    case WM_DESTROY:
           PostQuitMessage(0);
           break;

    default:
           return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return (0);
}
     
     
     
void 
report(HWND hwnd,int message,WPARAM wParam, LPARAM lParam){

                static int count;
                
                if(MsgTypeOk(message))  /* decide, if the message is to be displayed */
                {                             /* if so: Put it in array 'Messages' */
                                Messages[MsgIndLast].id = message;
                                Messages[MsgIndLast].wParam = wParam;
                                Messages[MsgIndLast].lParam = lParam;
                                Messages[MsgIndLast].count = ++count;
                                MsgIndLast = (MsgIndLast+1) % MSG_LINES;
                                if(MsgIndLast == MsgIndFirst)
                                   MsgIndFirst = (MsgIndFirst+1) % MSG_LINES;
                                InvalidateRect(hwnd, NULL, FALSE);      /* Redraw Client-area */
                                UpdateWindow(hwnd);
                }
}
     
     
     
/* Test, if the message belongs to the right type of messages. */
BOOL 
MsgTypeOk(int message) {
                      
                BOOL ok;
                
                ok = FALSE;                                
                /* we must exclude some messages, else we'll get stuck */
                if((message==WM_PAINT) 
                  || (message==WM_NCPAINT) 
                  || (message==WM_GETTEXT))
                  return FALSE;
                
                if(MsgType & MSG_KEY) {
                          ok = ok || ((message >= WM_KEYFIRST)
                                       &&
                                     (message < WM_KEYLAST));
                }
                if(MsgType & MSG_MOUSE) {
                          ok =  ok || (((message >= WM_MOUSEFIRST)
                                        &&
                                      (message < WM_MOUSELAST))
                                      ||
                                      ((message >= WM_NCMOUSEMOVE)
                                        &&
                                       (message < WM_NCMBUTTONDBLCLK))
                                      );
    }
    
                if(MsgType & MSG_WINDOW) {
                           ok = ok || (   (message == WM_MOVE)
                                        || (message == WM_SIZE)
                                        || (message == WM_WINDOWPOSCHANGED)
                                        || (message == WM_WINDOWPOSCHANGING)
                                        || (message == WM_VSCROLL)
                                        || (message == WM_HSCROLL)
                                        || (message == WM_KILLFOCUS)
                                        || (message == WM_SETFOCUS)
                                      );
                }
                
                if(MsgType & MSG_ALL) {
                          ok = !ok;
                }
                
                return ok;
}
     
     
     
void MsgPaint(HWND hwnd) {

          HDC hDC;
          PAINTSTRUCT PS;
          RECT Rect;
          int i;
          MSGCODE* msgp;
        
          hDC = BeginPaint(hwnd, &PS);  /* Get handle to display context */
        
          sprintf(MsgBuffer, "Displaying",MsgType);
          
          if(MsgType == 0) {
              strcat(MsgBuffer, " no ");
          }
          
          if(MsgType & MSG_ALL) {
                    strcat(MsgBuffer, " all ");
                    if(MsgType & (~MSG_ALL)) {
                        strcat(MsgBuffer, "messages except");
                    }
          }
        
          if(MsgType & MSG_KEY) {
                    strcat(MsgBuffer, " keyboard-");
          }
          
          if(MsgType & MSG_MOUSE) {
                    strcat(MsgBuffer, " mouse-");
          }
          
          if(MsgType & MSG_WINDOW) {
                    strcat(MsgBuffer, " window-");
                }
                
          strcat(MsgBuffer,"messages");
        
          SetRect(&Rect,0,0,1000,20);   /* Set dimensions of area where the headline */
                                        /* will be displayed */
          FillRect(hDC, &Rect, hBrush);      /* Clear this area */
          
          DrawText                      /* Display headline :                  */
                                (hDC,                       /* handle to display context           */
                                 MsgBuffer,                 /* Adress of string to display         */
                                 -1,                        /* -1 --> string is NULL-terminated    */
                                 &Rect,                         /* Rect, in which string is displayed  */
                                                                /* How string is to be displayed:      */
                                DT_LEFT |                   /* Horiz. left within 'Rect'           */
                                DT_TOP |                        /* Vert. at top of 'Rect'              */       
                                DT_SINGLELINE);                 /* Single line of text                 */       
        
          SetRect(&Rect, 0, 20, 1000, 40); /* Set area where mesgs will be displayed*/
          
          for(i=MsgIndFirst; i != MsgIndLast; i= (i+1) % MSG_LINES)
          {
                    /* Display next line */
                    OffsetRect(&Rect,0,20);     
                   
                    msgp = GetTwinMsgCode(Messages[i].id);      /* Get messagename */
                    sprintf(MsgBuffer," %5d:  %x, %s, %04x, %04x:%04x",
                               Messages[i].count,               /* Message-counter */
                               hwnd,                            /* Window handle   */
                               msgp->msgstr,                    /* Name of message */
                               Messages[i].wParam,              /* Parameter       */
                               HIWORD (Messages[i].lParam), 
                               LOWORD (Messages[i].lParam));
                
                    FillRect(hDC,&Rect,hBrush);                 /* Clear the line  */
                    
                    /* Display string in window. */
                    DrawText(hDC,               /* Handle to display context.      */
                             MsgBuffer,         /* Address of string to display.   */
                             -1,                /* -1 = string is NULL-terminated  */
                             &Rect,             /* Rect. in which string is displayed*/
                                                /* How string is to be displayed:  */
                             DT_LEFT |          /* Place horiz. left within 'Rect' */
                             DT_TOP |           /* Place vert. at top of 'Rect'.   */
                             DT_SINGLELINE);    /* Single line of text.            */
          }  
        
          EndPaint(hwnd, &PS);          /* Tell Windows area is painted    */
}                                                            


BOOL PASCAL 
WinMain(HANDLE hInst, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    HWND        hWnd;
    MSG         msg;


    LoadString(hInst,DLL_APP_TITLE,Title,256); 
    
    hInstance = hInst;

    hAccelTable = LoadAccelerators( hInstance, (LPSTR)"RESOURCE" );

    if(!hPrevInstance) {
	WNDCLASS WndClass;
       
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);
        WndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
        WndClass.lpszMenuName  = (LPSTR) Program;
        WndClass.lpszClassName = (LPSTR) Program;
        WndClass.hbrBackground = GetStockObject(WHITE_BRUSH);
        WndClass.hInstance     = hInst;
        WndClass.style       = CS_HREDRAW | CS_VREDRAW;
        WndClass.lpfnWndProc = MsgWndProc;
	WndClass.cbClsExtra  = 0;
	WndClass.cbWndExtra  = 0;

                                
        if( !RegisterClass(&WndClass) )
           return (FALSE);
    }
    else return(FALSE);

    hWnd = CreateWindow(Program,
        Title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        hInst,
        NULL);

    if (!hWnd) return (FALSE);
    
    hBrush = GetStockObject(WHITE_BRUSH); 

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MsgType = 0;
    MsgIndFirst = 0; 
    MsgIndLast  = 0;

    while (GetMessage(&msg, 0, 0, 0)) {
              if (TranslateAccelerator( hWnd, hAccelTable, (LPMSG)&msg ) == 0) {
                  TranslateMessage(&msg);
            DispatchMessage(&msg);
        }       
    }
    return (msg.wParam);
}
