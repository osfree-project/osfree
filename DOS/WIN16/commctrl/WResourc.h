/*  WResourc.h	1.4 
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

/*Control IDs for a Property Sheet */
#define IDAPPLY                         3
#define ID_HELP                         4
#define ID_BACK                         5
#define ID_NEXT                         6
#define ID_FINISH                       7

/*Toolbar symbols */
#define	IDB_STDSMALLCOLOR			    101
/*Note: these three IDs are already defined in commctrl.h. But Willows
  rc compiler would interpret them as strings. Being included here would
  somehow make them being interpreted as IDs, i.e they would be
  0x8001, 0x8004, 0x8005
 */
#define	IDB_STD_LARGE_COLOR			    1
#define	IDB_VIEW_SMALL_COLOR			    4
#define	IDB_VIEW_LARGE_COLOR			    5
#define IDC_CURSOR_DRAG				    105
/* Header control symbols */
#define IDC_DIVIDER				    106
#define IDC_DIVOPEN				    107

/* Drag Listbox symbols */
#define IDC_BTN_CUSTOMIZE_ADD           3
#define IDC_BTN_CUSTOMIZE_REMOVE        4
#define IDC_BTN_CUSTOMIZE_HELP          5
#define IDC_BTN_CUSTOMIZE_MOVEDOWN      6
#define IDC_BTN_CUSTOMIZE_MOVEUP        7
#define IDC_LBOX_CUSTOMIZE_LEFT         1069
#define IDC_LBOX_CUSTOMIZE_RIGHT        1070
#define IDC_BTN_CUSTOMIZE_RESET         1071
#define IDI_DRAGLBX_ARROW               150
