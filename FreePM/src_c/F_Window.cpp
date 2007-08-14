/* 
 $Id: F_Window.cpp,v 1.2 2002/09/06 12:29:43 evgen2 Exp $ 
*/
/* F_Window.cpp */
/* functions of class FreePM_Window */
/* ver 0.01 3.09.2002       */

#include <FreePM.hpp>
#include <malloc.h>

/* создание окна */
/* Changing FreePM_Window to FPM_Window. */ 
 FPM_Window::FPM_Window
                  (HWND hwndParent,  /*  Parent-window handle. */
                   PCSZ  _pszClass,   /*  Registered-class name. */
                   PCSZ  _pszName,    /*  Window text. */
                   ULONG _flStyle,    /*  Window style. */
                   LONG _x,          /*  x-coordinate of window position. */
                   LONG _y,          /*  y-coordinate of window position. */
                   LONG _nx,          /*  Width of window, in window coordinates. */
                   LONG _ny,          /*  Height of window, in window coordinates. */
                   HWND hwndOwner,   /*  Owner-window handle. */
                   HWND hwndInsertBehind, /*  Sibling-window handle. */
                   ULONG _rid,       /*  Window resource identifier. */
                   PVOID pCtlData,   /*  Pointer to control data. */
                   PVOID pPresParams)/*  Presentation parameters. */
{  int i;
   x = _x;
   y = _y;
   nx = _nx;
   /* ny - _ny; What for? */
   /* Maybe this: */
   ny = _ny;
   flStyle = _flStyle;
//   class_type = 0;
   pszClass = _pszClass;
   pszName  = _pszName;
   rid = _rid;

}

///* добавить окно к десктопу */
//int FreePM_DeskTop::AddWindow(int x, int y, int nx, int ny, int bytesPerPixel)
//{
//
//   return 0;
//}
//
///* удалить окно с десктопа */
//int FreePM_DeskTop::DelWindow(int ind)
//{
//
//   return 0;
//}


int FPM_Window::proc( HWND hwndWnd, ULONG ulMsg, MPARAM mpParm1,MPARAM mpParm2 )
{
 //todo
  return NULL;
}  


