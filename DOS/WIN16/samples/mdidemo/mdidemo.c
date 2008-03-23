/*
 *      MDIDemo.c
 *
 *      @(#)mdidemo.c	1.1 5/3/96 16:27:19 /users/sccs/src/samples/mdidemo/s.mdidemo.c
 * 
 *      MDI Windows Sample Application
 *
 *      Demonstrates the following:
 *
 *      - use of resource and module definition compilers.
 *      - registering classes, and creating windows.
 *      - standard message loop and message dispatch logic.
 *      - use of resources, menues, bitmaps, stringtables and accelerators.
 *
 *      - use of the Multiple Document Interface
 *      - Creating and destroying MDI child windows
 *
 *      - calling the ShellAbout library call.
 */

#include "mdidemo.h"
     
/* global variables */
/*------------------*/

HINSTANCE       hInstance;

/* menuhandles and handles to submenus where the windowlist will be appended */
HMENU           hInitMenu, hMdiMenu;
HMENU           hWL_InitMenu, hWL_MdiMenu;

/* title of the application */
char            title[256];

int PASCAL 
WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND        hWnd, hWndClient;
    HACCEL      hAccelTable;
    MSG         msg;

    /* get the title of the application */    
    LoadString(hInst,MDI_APP_TITLE,title,256);
        
    hInstance = hInst;
    /* and the accelerators */
    hAccelTable = LoadAccelerators( hInstance, (LPSTR)"RESOURCE" );

    /* now we have to register the window class for the MDI-framewindow */
    if (!hPrevInstance)
    {
        WNDCLASS wc;
        
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_WILLOWS));
        wc.lpszMenuName  = NULL;        /* Menuhandle will be given in CreateWindow */
        wc.lpszClassName = (LPSTR) "FrameWindow";
        wc.hbrBackground = COLOR_APPWORKSPACE+1;
        wc.hInstance     = hInst;
        wc.style       = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = FrameWndProc;
        wc.cbWndExtra  = 0;
        wc.cbClsExtra  = 0;
        
        if(!RegisterClass(&wc))
          return 0;
    /* ... and the windowclass for the MDI childwindow */
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        wc.lpszMenuName  = NULL;        /* a childwindow has no menu */
        wc.lpszClassName = (LPSTR) "ChildWindow";
        wc.hbrBackground = GetStockObject(WHITE_BRUSH);
        wc.hInstance     = hInst;
        wc.style       = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = ChildWndProc;
        wc.cbWndExtra  = sizeof(LOCALHANDLE);   /* allocate extra space */
        wc.cbClsExtra  = 0;
        
        if(!RegisterClass(&wc))
          return 0;
    }
    /* now get the handles to the menus */
    hInitMenu = LoadMenu(hInst, "InitMenu");
    hMdiMenu = LoadMenu(hInst, "MdiMenu");
    
    /* and the handles to the submenus where the windowlist will be shown */
    hWL_InitMenu = GetSubMenu(hInitMenu, WLPOS_INITMENU);
    hWL_MdiMenu = GetSubMenu(hMdiMenu, WLPOS_MDIMENU);

    /* now we can create the framewindow */
    hWnd = CreateWindow("FrameWindow",
                        title,
                        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        0,
                        hInitMenu,
                        hInst,
                        NULL);
    if (!hWnd)
       return (0);
       
    hWndClient = GetWindow(hWnd, GW_CHILD);
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    /* the message loop is modified for an MDI application */
    while (GetMessage(&msg, 0, 0, 0)) 
    {     
      if(!TranslateMDISysAccel(hWndClient, &msg) &&
         !TranslateAccelerator( hWnd, hAccelTable, (LPMSG)&msg )) 
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      } 
    }
    
    /* destroy unattached menu */
    DestroyMenu(hMdiMenu);
    return (msg.wParam);
}


long FAR PASCAL __export 
FrameWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{     
  HICON  hIcon;  
  static HWND hWndClientWindow;  
  static UINT ChildCount = 0;
  CLIENTCREATESTRUCT ccsCreate;
  FARPROC lpfnEnum;
  HWND hWndChild;
  MDICREATESTRUCT mcsCreate;
  
                           
  switch (message)
  {
    case WM_CREATE:         
        /* when the framewindow is created we also create the clientwindow */
        ccsCreate.hWindowMenu = hWL_InitMenu;    /* handle to submenu for windowlist */
        ccsCreate.idFirstChild = IDM_FIRSTCHILD; /* identifier for first created childwindow */
        hWndClientWindow = CreateWindow(
                            "MDICLIENT",    /* predefined window class for MDI-clientwindow */
                            NULL,
                            WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
                            0,
                            0,
                            0,
                            0,
                            hWnd,
                            1,
                            hInstance,
                            (LPSTR) &ccsCreate);
        break;
        
    case WM_COMMAND:
        switch (wParam)
        {
            case IDM_ABOUT:
                hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_WILLOWS));
                ShellAbout(hWnd, "MDIDemo", title, hIcon);
                break;
            
            case IDM_EXIT:
                SendMessage(hWnd, WM_CLOSE, 0, 0L);
                break;
    
            /* the messages for creating, closing or arranging the childwindows */
            case IDM_NEW:   /* create a new childwindow */
              {                 
                char buffer[100];
               
                ChildCount++;
                wsprintf(buffer, "Child #%d", ChildCount);
            
                mcsCreate.szClass   = "ChildWindow";
                mcsCreate.szTitle   = buffer;
                mcsCreate.hOwner    = hInstance;
                mcsCreate.x         = CW_USEDEFAULT;
                mcsCreate.y         = CW_USEDEFAULT;
                mcsCreate.cx        = CW_USEDEFAULT;
                mcsCreate.cy        = CW_USEDEFAULT;
                mcsCreate.style     = 0;
                mcsCreate.lParam    = 0L;
    
                hWndChild = (HWND) SendMessage(hWndClientWindow, WM_MDICREATE, 0,
                                                (long) (LPMDICREATESTRUCT) &mcsCreate);
                break;
              } 
            case IDM_CLOSE: /* close the active childwindow */
                hWndChild = LOWORD(SendMessage(hWndClientWindow, WM_MDIGETACTIVE, 0, 0L));
                
                if(SendMessage(hWndChild, WM_QUERYENDSESSION, 0, 0L))
                   SendMessage(hWndClientWindow, WM_MDIDESTROY, hWndChild, 0L);
                break;                           
                
            case IDM_TILE:
                SendMessage(hWndClientWindow, WM_MDITILE, 0, 0L);
                break;
                
            case IDM_CASCADE:
                SendMessage(hWndClientWindow, WM_MDICASCADE, 0, 0L);
                break;
                
            case IDM_ARRANGE:
                SendMessage(hWndClientWindow, WM_MDIICONARRANGE, 0, 0L);
                break;
                
            case IDM_CLOSEALL:
                lpfnEnum = MakeProcInstance((FARPROC) CloseChildProc, hInstance);
                EnumChildWindows(hWndClientWindow, lpfnEnum, 0L);
                FreeProcInstance(lpfnEnum);
                break;
                
            default:    /* all other WM_COMMAND messages: pass to active child */
                hWndChild = LOWORD(SendMessage(hWndClientWindow, WM_MDIGETACTIVE, 0, 0L));
                if(IsWindow( hWndChild))
                  SendMessage(hWndChild, WM_COMMAND, wParam, lParam);
                return DefFrameProc(hWnd, hWndClientWindow, message, wParam, lParam);
        }
        break;        
        
     case WM_QUERYENDSESSION:
     case WM_CLOSE:         /* attempt to close all children */
        SendMessage(hWnd, WM_COMMAND, IDM_CLOSEALL, 0L);
        if(!GetWindow(hWndClientWindow, GW_CHILD))
          return DefFrameProc(hWnd, hWndClientWindow, message, wParam, lParam);
        break;

     case WM_DESTROY:
        PostQuitMessage(0);
        break;
                
    default:    /* unprocessed messages go to DefFrameProc ! */
        return (DefFrameProc(hWnd, hWndClientWindow, message, wParam, lParam));
  }
    return (0);
}

long FAR PASCAL _export ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{                    
    static HWND hWndFrame;
    static HWND hWndClientWindow;
    
    switch(message)
    {                                                           
        case WM_CREATE:
            /* get some window handles */
            hWndClientWindow= GetParent(hWnd); 
            hWndFrame       = GetParent(hWndClientWindow);
            break;
    
        case WM_MDIACTIVATE:      /* gaining/losing focus --> setting menus */
            if(wParam == TRUE)    /* gaining focus --> set the MdiMenu */
            {
              SendMessage(hWndClientWindow, WM_MDISETMENU, 0, 
                            MAKELONG(hMdiMenu, hWL_MdiMenu));
            }
            else
            {                     /* losing focus -- set InitMenu */
              SendMessage(hWndClientWindow, WM_MDISETMENU, 0,
                            MAKELONG(hInitMenu, hWL_InitMenu));
            }
            DrawMenuBar(hWndFrame);   /* Draw the new menubar */
            break;   
          
        case WM_QUERYENDSESSION:
        case WM_CLOSE:   
          { 
            char buffer[100];
            
            GetWindowText(hWnd, buffer, sizeof(buffer));
            if(MessageBox(hWnd, "Close window?",buffer, MB_ICONQUESTION|MB_OKCANCEL) == IDOK)
              return DefMDIChildProc(hWnd, message, wParam, lParam);
            break;  
          }

        default: 
          return DefMDIChildProc(hWnd, message, wParam, lParam);  
    }
    return 0;      
}

BOOL FAR PASCAL _export CloseChildProc(HWND hWnd, LONG lParam)
{
    if(GetWindow(hWnd, GW_OWNER))
      return TRUE;
      
    SendMessage(GetParent(hWnd), WM_MDIRESTORE, hWnd, 0L);
    
    if(!SendMessage(hWnd, WM_QUERYENDSESSION, 0, 0L))
      return TRUE;
    
    SendMessage(GetParent(hWnd), WM_MDIDESTROY, hWnd, 0L);
    return TRUE;
}
