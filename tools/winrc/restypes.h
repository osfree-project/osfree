/*************************************************************************

	restypes.h	1.2
	data structures used internally by resource compiler
  
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

**************************************************************************/

/* MFC 4.x internal resource types */

#ifndef RT_DLGINIT
#define RT_DLGINIT	MAKEINTRESOURCE(240)
#endif	/* RT_DLGINIT */

#ifndef RT_TOOLBAR
#define RT_TOOLBAR	MAKEINTRESOURCE(241)
#endif	/* RT_DLGINIT */

/* All the structures basically the same as defined in Resources.h
 * except for the 'next' pointer. 
 */

typedef struct tagACCEL_NODE {
  ACCELENTRY	data;
  struct	tagACCEL_NODE	*next;
} ACCEL_NODE;


typedef struct tagMENU_NODE {
  MENUDATA data;
  WORD    index; /* used to give names for structures */
  struct  tagMENU_NODE       *next;
  struct  tagMENU_NODE       *child;
} MENU_NODE;


typedef struct tagCONTROL_NODE {
  CONTROLDATA	data;
  struct tagCONTROL_NODE *next;
} CONTROL_NODE;

typedef struct tagDIALOG_NODE {
  DIALOGDATA  data;
  CONTROL_NODE	*controlinfo;
  struct	tagDIALOG_NODE *next;
} DIALOG_NODE;

typedef struct tagCURSORDIR_NODE {
  CURSORDIRENTRY	data;
  struct	tagCURSORDIR_NODE *next;
} CURSORDIR_NODE;

typedef struct tagICONDIR_NODE {
  ICONDIRENTRY	data;
  struct	tagICONDIR_NODE *next;
} ICONDIR_NODE;


typedef struct  tagCURSOR_NODE{
  CURSORIMAGE	data;
  struct	tagCURSOR_NODE *next;
} CURSOR_NODE;


typedef struct  tagBITMAP_NODE{
  BITMAPIMAGE	data;
  struct	tagBITMAP_NODE *next;
} BITMAP_NODE;

typedef struct  tagICON_NODE{
  ICONIMAGE	data;
  struct	tagICON_NODE *next;
} ICON_NODE;

typedef struct tagDLGINIT_NODE {
  DLGINITDATA	data;
  struct	tagDLGINIT_NODE *next;
} DLGINIT_NODE;

typedef struct tagTOOLBAR_NODE {
  TOOLBARDATA	data;
  struct	tagTOOLBAR_NODE *next;
} TOOLBAR_NODE;

typedef	char	*STRING_DATA[16];

typedef	struct	tagNAMEINFO_NODE {
  NAMEINFO	data;
  struct tagNAMEINFO_NODE *next;
} NAMEINFO_NODE;

typedef struct NAMEENTRY_NODE {
  NAMEENTRY	data;
  struct	NAMEENTRY_NODE *next;
} NAMEENTRY_NODE;

extern TYPEINFO *resource_table;

#define	MAX_TYPES	32

extern BYTE *new_ptr;
extern BYTE *new_table;

/* Functions defined in resources.c and not declared in resfunc.h */
STRING_DATA *find_string_segment(int id);

/* Functions defined in util.c */

WORD get_word(BYTE *ptr);
DWORD get_dword(BYTE *ptr);
WORD bit_to_rgb(WORD bitcount);
void pascal_to_asciiz(char *str);
void asciiz_to_pascal(char *str);
MENU_NODE *get_child(MENU_NODE **root);
char *search_namelist_type(WORD res_type);
void incr_res_count(char *typename, NAMEINFO_NODE *info);
void get_bmp_info_header(BITMAPINFOHEADER *bmp_info_header, BYTE *ptr);
