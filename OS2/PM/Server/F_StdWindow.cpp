/*
 $Id: F_StdWindow.cpp,v 1.2 2002/11/10 15:20:03 evgen2 Exp $
*/
/* F_StdWindow.cpp */
/* functions of class STD_Window */
/* ver 0.02 9.10.2002       */

#include "FreePM.hpp"

#include <malloc.h>


/* создание стандартного окна */
int STD_Window::CreateSTD_Window(HWND hwndParent,  /*  Parent-window handle. */
               ULONG _flStyle,           /*  Frame Window style. */
               PULONG pflCreateFlags,
               PSZ pszClientClass,       /* Client Class */
               PSZ pszTitle,             /* Title */
               ULONG styleClient,        /* Client Window style. */
               HMODULE hmod,             /* resource module */
               ULONG idResources,        /*  Window resource identifier. */
               PHWND phwndClient)        /*  return: client window hwnd */
{  int i;
   FPM_Window *pw;
   x = 10;
   y = 10;
   nx = 256;
   ny = 256;
   flStyle = _flStyle;
//   class_type = 0;
   pszClass = pszTitle;
   rid = idResources;
//   pszName  = _pszName;


//debug(3, 0)(__FUNCTION__": %s\n", pszTitle);

// todo: determinate titlebar size
//warp style: frame border = 1+1+2, titlebar = 2+20 (1border)  + 22 menu if present
//frame window
        CreateFPM_Window(hwndParent,  /*  Parent-window handle. */
                        WC_FRAME, /*  Registered-class name. */
                        NULL,     /*  Window text. */
                        flStyle, /*  Window style. */
                        x,        /*  x-coordinate of window position relative to the origin of the parent window */
                        y,        /*  y-coordinate of window position relative to the origin of the parent window */
                        nx,       /*  Width of window, in window coordinates. */
                        ny,       /*  Height of window, in window coordinates. */
                        hwndParent,   /*  Owner-window handle. */
                        NULL,     /*  Sibling-window handle. */
                        NULL,     /*  Window resource identifier. */
                        NULL,     /*  Pointer to control data. */
                        NULL);    /*  Presentation parameters. */

//   FPM_Window Client;
    pClient = new(FRAME_Window);
    pClient->CreateFPM_Window(handle,  /*  Parent-window handle. */
                        pszClientClass,    /*  Registered-class name. */
                        NULL,     /*  Window text. */
                        styleClient,     /*  Window style. */
                        4,      /*  x-coordinate of window position. */
                        4,      /*  y-coordinate of window position. */
                        nx-4-22,  /*  Width of window, in window coordinates. */
                        ny-4-22,  /*  Height of window, in window coordinates. */
                        handle,   /*  Owner-window handle. */
                        NULL,     /*  Sibling-window handle. */
                        FID_CLIENT,     /*  Window resource identifier. */
                        NULL,     /*  Pointer to control data. */
                        NULL);    /*  Presentation parameters. */

//   FPM_Window SysMenu;
    pSysMenu = new(FRAME_Window);
    pSysMenu->CreateFPM_Window(handle,  /*  Parent-window handle. */
                        WC_MENU,    /*  Registered-class name. */
                        NULL,     /*  Window text. */
                        WS_VISIBLE,     /*  Window style. */
                        4,       /*  x-coordinate of window position. */
                        ny-4-22, /*  y-coordinate of window position. */
                        22,  /*  Width of window, in window coordinates. */
                        22,  /*  Height of window, in window coordinates. */
                        handle,   /*  Owner-window handle. */
                        NULL,     /*  Sibling-window handle. */
                        FID_SYSMENU, /*  Window resource identifier. */
                        NULL,     /*  Pointer to control data. */
                        NULL);    /*  Presentation parameters. */

//todo:determinate size of menu (yet 22*3 x 22)
//   FPM_Window MinMax Menu;
    pMinMaxMenu = new(FRAME_Window);
    pMinMaxMenu->CreateFPM_Window(handle,  /*  Parent-window handle. */
                        WC_MENU,   /*  Registered-class name. */
                        NULL,      /*  Window text. */
                        WS_VISIBLE,/*  Window style. */
                        nx-4-22*3, /*  x-coordinate of window position. */
                        ny-4-22,   /*  y-coordinate of window position. */
                        22,  /*  Width of window, in window coordinates. */
                        22,  /*  Height of window, in window coordinates. */
                        handle,   /*  Owner-window handle. */
                        NULL,     /*  Sibling-window handle. */
                        FID_MINMAX, /*  Window resource identifier. */
                        NULL,     /*  Pointer to control data. */
                        NULL);    /*  Presentation parameters. */
//   FPM_Window Menu;
    pMenu = new(FRAME_Window);
    pMenu->CreateFPM_Window(handle,  /*  Parent-window handle. */
                        WC_MENU ,   /*  Registered-class name. */
                        pszTitle,      /*  Window text. */
                        WS_VISIBLE,/*  Window style. */
                        22+1, /*  x-coordinate of window position. */
                        ny-4-22-2,   /*  y-coordinate of window position. */
                        nx-4-22*4,  /*  Width of window, in window coordinates. */
                        22,  /*  Height of window, in window coordinates. */
                        handle,   /*  Owner-window handle. */
                        NULL,     /*  Sibling-window handle. */
                        FID_MENU, /*  Window resource identifier. */
                        NULL,     /*  Pointer to control data. */
                        NULL);    /*  Presentation parameters. */
//   FPM_Window TitleBar;
    pTitleBar = new(FRAME_Window);
    pTitleBar->CreateFPM_Window(handle,  /*  Parent-window handle. */
                        WC_TITLEBAR ,   /*  Registered-class name. */
                        pszTitle,      /*  Window text. */
                        WS_VISIBLE,/*  Window style. */
                        22+1, /*  x-coordinate of window position. */
                        ny-4-22-2,   /*  y-coordinate of window position. */
                        nx-4-22*4,  /*  Width of window, in window coordinates. */
                        22,  /*  Height of window, in window coordinates. */
                        handle,   /*  Owner-window handle. */
                        NULL,     /*  Sibling-window handle. */
                        FID_TITLEBAR, /*  Window resource identifier. */
                        NULL,     /*  Pointer to control data. */
                        NULL);    /*  Presentation parameters. */
//todo:    FPM_Window VertScroll;
//todo:    FPM_Window HorizScroll;


    return 0;
}


int FPM_Window::proc( PQMSG pqmsg)
{
 //todo
  return NULL;
}

int FPM_Window::Draw(HPS hps)
{
//debug(3, 0)("DrawTest:"__FUNCTION__"\n");
 //todo
  return NULL;
}

