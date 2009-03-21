/*    
	printres.c	1.9
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


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

#include <stdio.h>
#include <string.h>

#include "windows.h"
#include "Resources.h"
#include "rc.h"
#include "restypes.h"
 
#define	CHARS_PER_LINE		60

extern WORD  bit_to_rgb( WORD bit_count);

void parse_module(void);
void print_module(void);

static char    *info_name_tbl[] = {
    "cursor_info",
    "bitmap_info",
    "icon_info",
    "menu_info",
    "dialog_info",
    "string_info",
    "fontdir_info",
    "font_info",
    "accel_info",
    "rcdata_info",
    "",
    "cursor_dir_info",
    "",
    "icon_dir_info",
    "nametbl_info",
    ""
};
 
static char userName[255];

void 
print_last_entry(int num)
{
        COUTPUT(("\t{"));
        while(--num)
                COUTPUT(("0, "));
        COUTPUT(("0 }\n"));
}
 

void
print_asciiz_string(char *str)
{
	int	i;
	int	counter = 0;
        if ((str == (char *)NULL) || (str[0] == 0)) {
                COUTPUT(("0 "));
                return;
        }
        COUTPUT(("\""));
        for (i = 0; str[i]; i++) {
                if (++counter == CHARS_PER_LINE)  {
                        COUTPUT(("\\\n"));
                        counter = 0;
                }
                if (str[i] == '\n')
                        COUTPUT(("\\n"));
                else if (str[i] == '\t')
                        COUTPUT(( "\\t"));
                else if (str[i] == '\"')
                        COUTPUT(( "\\\""));
                else
                        COUTPUT(( "%c", str[i]));
        }
        COUTPUT(( "\""));

}

void
print_num_or_str(char *str)
{
	if (HIWORD(str) == 0)
		COUTPUT(( "(char *) 0x%04x", (int)str));
	else
		print_asciiz_string(str);
}

void
print_bmp_hdr(char *title, int id, BITMAPINFOHEADER *ptr)
{
	COUTPUT(( "static BITMAPINFOHEADER %s_bmp_hdr_%d[] = {\n\t{", title, id));
	COUTPUT(( " 0x%08lx,\n\t0x%08lx,\n\t0x%08lx,\n",
			ptr->biSize, ptr->biWidth, ptr->biHeight));
	COUTPUT(( "\t0x%04lx,\n\t0x%04lx,\n\t0x%08lx,\n",
			ptr->biPlanes, ptr->biBitCount, ptr->biCompression));
	COUTPUT(( "\t0x%08lx,\n\t0x%08lx,\n\t0x%08lx,\n",
			ptr->biSizeImage, ptr->biXPelsPerMeter, ptr->biYPelsPerMeter));
	COUTPUT(( "\t0x%08lx,\n\t0x%08lx }\n}\n;\n\n",
			ptr->biClrUsed, ptr->biClrImportant));
}

void
print_rgb_quad(char *title, int id, RGBQUAD *ptr, int count)
{
	int	i;

        COUTPUT(( "static RGBQUAD %s_rgb_quad_%d[] = {\n",title, id));
        for (i = 0; i < count; i++) {
                COUTPUT(("\t{ 0x%02x, 0x%02x, 0x%02x, 0x00 },\n",
                        (BYTE)ptr->rgbBlue, (BYTE)ptr->rgbGreen, (BYTE)ptr->rgbRed));
                ptr++;
        }
        print_last_entry(4);
        COUTPUT(("};\n\n"));
}
 
void
print_bits(char *title, int id, BYTE *ptr, int count)
{
        int    j;
 
        COUTPUT(( "static BYTE  %s_%d[] = {\n", title, id));
        for (j = 0; j < count; j++) {
                if ( (j % 8) == 0)
                        COUTPUT(("\n"));
                COUTPUT(("0x%02x",ptr[j]));
                if (j == (count - 1))
                        COUTPUT(("\n"));
                else
                        COUTPUT((", "));
        }
        COUTPUT(("\n};\n\n"));
}

void
print_str_bits(char *title, WORD id, BYTE *ptr, WORD count)
{
        WORD    i;
	int	line_length = 0;

        COUTPUT(( "static char  %s_data_%d[] = \"\\\n", title, id));
        for (i = 0; i < count; i++) {
		if (ptr[i] >= 32 && ptr[i] < 126) {
			if ((ptr[i] == '"') || 
			    (ptr[i] == '\'') || 
			    (ptr[i] == '\\')) {
				COUTPUT(("\\%c",ptr[i]));
				line_length += 2;
			} else {
				COUTPUT(("%c",ptr[i]));
				line_length += 1;
			}
		} else {
			COUTPUT(("\\%03o",ptr[i]));
			line_length += 4;
		}
		if (line_length >= 65) {
			COUTPUT(("\\\n"));
			line_length = 0;
		}
        }
	COUTPUT(("\";\n\n"));
}

void print_cursor_res(int i, NAMEINFO *info)
{
  CURSOR_NODE *node;
  CURSORIMAGE *image;
  int count;

  node = (CURSOR_NODE *)info->rcsdata;
  image = &(node->data);
  print_bmp_hdr("cursor", i, image->bmp_hdr);
  count = (int)bit_to_rgb(image->bmp_hdr->biBitCount);
  print_rgb_quad("cursor", i, image->rgb_quad, count);
  count = (image->bmp_hdr->biHeight * image->bmp_hdr->biWidth)/16;
  print_bits("cursor_xor_bits", i, (BYTE *)image->xor_mask, count);
  print_bits("cursor_and_bits", i, (BYTE *)image->and_mask, count);
  COUTPUT(( "static CURSORIMAGE cursor_data_%d = {\n",i));
  COUTPUT(("\t0x%04x, 0x%04x, ",image->wHotSpotX, image->wHotSpotY)); 
  COUTPUT(("cursor_bmp_hdr_%d, cursor_rgb_quad_%d, ", i, i));
  COUTPUT(("cursor_xor_bits_%d, cursor_and_bits_%d", i, i));
  COUTPUT(("};\n\n"));
}

void print_bitmap_res(int i, NAMEINFO *info)
{
  BITMAP_NODE *node;
  BITMAPIMAGE *image;
  int count;

  node = (BITMAP_NODE *)info->rcsdata;
  image = &(node->data);
  print_bmp_hdr("bitmap", i, image->bmp_hdr);
  count = bit_to_rgb(image->bmp_hdr->biBitCount);
  print_rgb_quad("bitmap", i, image->rgb_quad, count);
  count = image->bmp_hdr->biSizeImage;
  print_bits("bitmap_bits", i, (BYTE *)image->bitmap_bits, count);
  COUTPUT(( "static BITMAPIMAGE bitmap_data_%d = {\n",i));
  COUTPUT(("bitmap_bmp_hdr_%d, bitmap_rgb_quad_%d, ", i, i));
  COUTPUT(( "bitmap_bits_%d",i));
  COUTPUT(("};\n\n"));
}

void print_icon_res(int i, NAMEINFO *info)
{
  ICON_NODE *node;
  ICONIMAGE *image;
  int count;

  node = (ICON_NODE *)info->rcsdata;
  image = &(node->data);
  print_bmp_hdr("icon", i, image->icon_bmp_hdr);
  count = bit_to_rgb(image->icon_bmp_hdr->biBitCount);
  print_rgb_quad("icon", i, image->icon_rgb_quad, count);
  count = (int)(image->icon_bmp_hdr->biHeight * image->icon_bmp_hdr->biWidth * image->icon_bmp_hdr->biBitCount)/16;
  print_bits("icon_xor_bits", i, (BYTE *)image->icon_xor_mask, count);
  count = (image->icon_bmp_hdr->biHeight * image->icon_bmp_hdr->biWidth)/16;
  print_bits("icon_and_bits", i, (BYTE *)image->icon_and_mask, count);
  COUTPUT(( "static ICONIMAGE icon_data_%d = {\n",i));
  COUTPUT(("icon_bmp_hdr_%d, icon_rgb_quad_%d, ", i, i));
  COUTPUT(( "icon_xor_bits_%d, icon_and_bits_%d",i,i));
  COUTPUT(("};\n\n"));
}

/*
 * prints one level of menu list. the refrence to the pop up menus is
 * done by the counter stored at each node.
 */

void
print_menu_list(MENU_NODE *node)
{
  MENUDATA *current;
	
  COUTPUT(( "static MENUDATA m_%x[] = {\n", (int)node));
  while (node)
    {
      current = &(node->data);
      COUTPUT(( "MENUITEM( 0x%04x, 0x%04x, ", current->menuflag, current->menuid)); 
      print_asciiz_string(current->menuname);
      COUTPUT((", "));
      if (node->child)
	COUTPUT(( "m_%x )", (int)(node->child)));
      else 
	COUTPUT(( "0 )"));  
      node = node->next;
      COUTPUT((",\n"));
      if (current->menuflag & MF_END)
	break;
    };
  print_last_entry(4);
  COUTPUT(( "};\n\n"));
}

/* 
 * recursively calls its children. A menu can be printed only after 
 * all its children are printed. This avoids forward refrences.
 */

void
do_print(MENU_NODE *root)
{
  MENU_NODE *current;
  
  current = root;
  while (current)
    {
      if (current->child)
	do_print(current->child);
      current = current->next;
    }
  print_menu_list(root);
}

void print_menu_res(int i, NAMEINFO *info)
{
  do_print((MENU_NODE *)(info->rcsdata));
}

void print_menu_name(int i, NAMEINFO *info, char *title)
{
  COUTPUT(( "\t{ (LPBYTE)%s_%x, ", title, (int)(info->rcsdata)));
}

void
print_control_data(CONTROL_NODE *node, int id, int count)
{
  COUTPUT(( "static CONTROLDATA control_data_%d[] = {\n", id));

  while (count--)
    {
      CONTROLDATA *ptr = &(node->data);
      COUTPUT(( "\tCONTROLITEM( 0x%04x, 0x%04x, 0x%04x, 0x%04x, ",
	ptr->x, ptr->y, ptr->cx, ptr->cy));
      COUTPUT(( "0x%04x, 0x%08lx, 0x%08lx, ",
	ptr->wID, ptr->lStyle, ptr->dwExtStyle));    
      print_asciiz_string(ptr->szClass);	
      COUTPUT(( ", "));
      print_num_or_str(ptr->szText);
      COUTPUT(( " ),\n"));
      node = node->next;
    } 
  print_last_entry(8);
  COUTPUT(( "};\n\n"));
}

void print_dialog_res(int i, NAMEINFO *info)
{
  DIALOG_NODE *dlg_node;
  DIALOGDATA *dlg_data;

  dlg_node = (DIALOG_NODE *)(info->rcsdata);
  dlg_data = &(dlg_node->data);
  print_control_data(dlg_node->controlinfo, i, dlg_data->bNumberOfItems);
  COUTPUT(( "static DIALOGDATA dialog_data_%d = \n\t{", i));
  COUTPUT(( " 0x%08lx, 0x%08lx, 0x%04x, ",
	dlg_data->lStyle, dlg_data->dwExtStyle, dlg_data->bNumberOfItems));
  COUTPUT(( "0x%04x, 0x%04x, 0x%04x, 0x%04x, ",
	dlg_data->x, dlg_data->y, dlg_data->cx, dlg_data->cy));
  print_num_or_str(dlg_data->szMenuName);
  COUTPUT((", "));
  print_num_or_str(dlg_data->szClassName);
  COUTPUT((", "));
  print_asciiz_string(dlg_data->szCaption);
  COUTPUT((", "));
  COUTPUT(( "0x%04x, ", dlg_data->wPointSize));
  print_num_or_str(dlg_data->szFaceName);
  COUTPUT((", "));
  COUTPUT(( "control_data_%d };\n\n", i));
}

void print_string_res(int i, NAMEINFO *info)
{
  STRING_DATA *str_data;
  int j;

  str_data = (STRING_DATA *) info->rcsdata;
  COUTPUT(( "static STRINGENTRY string_data_%d[16] = {\n",i));
  for (j = 0; j < 16; j++)
    {
      COUTPUT(("\t"));
      print_asciiz_string((*str_data)[j]);
      if (j == 15)
	COUTPUT(("\n};\n\n"));
      else
	COUTPUT((",\n"));
    }
}

void print_fontdir_res(int i, NAMEINFO *info)
{
}

void print_font_res(int i, NAMEINFO *info)
{
}

void print_accel_res(int i, NAMEINFO *info)
{
  ACCEL_NODE *nentry;
  ACCELENTRY *accel_data;

  nentry = (ACCEL_NODE *) info->rcsdata;
  COUTPUT(("static ACCELENTRY accel_data_%d[] = {\n",i));
  while(nentry)
    {
      accel_data = &(nentry->data);
      if (nentry->next == NULL)
        accel_data->fFlags |= 0x80;
      COUTPUT(("\t{ 0x%02x, 0x%04x, 0x%04x },\n",
	       (BYTE)accel_data->fFlags, accel_data->wEvent,
	       accel_data->wId));
      if (accel_data->fFlags & 0x80)
	break;
      nentry = nentry->next;
    } 
  print_last_entry(3);
  COUTPUT(( "};\n\n"));
}

void print_rcdata_res(int i, NAMEINFO *info)
{
  print_str_bits("rc", i, (BYTE *)(info->rcsdata), info->rcslength);
}

void print_user_res(int i, NAMEINFO *info)
{
  print_str_bits(userName, i, (BYTE *)(info->rcsdata), info->rcslength);
}

void print_group_icon_res(int i, NAMEINFO *info)
{
  ICONDIR_NODE *nentry;
  ICONDIRENTRY *entry;

  nentry = (ICONDIR_NODE *) info->rcsdata;
  COUTPUT(( "static ICONDIRENTRY icon_dir_data_%d[] = {\n", i));
  while (nentry)
    {
      entry = &(nentry->data);
      if (entry->wOrdinalNumber == 0)
	break;
      COUTPUT(("\t{ 0x%02x, 0x%02x, 0x%02x, ", (BYTE)entry->bWidth, (BYTE) entry->bHeight, (BYTE)entry->bColorCount));
      COUTPUT(( "0x%04x, 0x%04x, ", entry->wPlanes, entry->wBitsPerPel));
      COUTPUT(( "0x%08x, 0x%04x },\n", entry->dwBytesInRes, entry->wOrdinalNumber));
      nentry = nentry->next;
    }
  print_last_entry(7);
  COUTPUT(( "};\n\n"));
}

void print_group_cursor_res(int i, NAMEINFO *info)
{
  CURSORDIR_NODE *nentry;
  CURSORDIRENTRY *entry;

  nentry = (CURSORDIR_NODE *) info->rcsdata;
  COUTPUT(( "static CURSORDIRENTRY cursor_dir_data_%d[] = {\n", i));
  while (nentry)
    {
      entry = &(nentry->data);
      if (entry->wOrdinalNumber == 0)
	break;
      COUTPUT(("\t{ 0x%02x, 0x%02x, ", (BYTE)entry->wWidth, (BYTE) entry->wHeight));
      COUTPUT(( "0x%04x, 0x%04x, ", entry->wPlanes, entry->wBitsPerPel));
      COUTPUT(( "0x%08x, 0x%04x },\n", entry->dwBytesInRes, entry->wOrdinalNumber));
      nentry = nentry->next;
  }
  print_last_entry(6);
  COUTPUT(( "};\n\n"));
}

void print_dlginit_res(int i, NAMEINFO *info)
{
  DLGINIT_NODE *dlginit_node;
  DLGINITDATA *dlginit_data;
  WORD j, k;

  dlginit_node = (DLGINIT_NODE *)(info->rcsdata);
  dlginit_data = &(dlginit_node->data);

  COUTPUT(("static BYTE u240_data_%d[] =\n", i));
  COUTPUT(("{\n"));
  for (j = 0; j < dlginit_data->wDataSize; j += k)
  {
    COUTPUT(("\t"));
    for (k = 0; (k < 16) && (j + k < dlginit_data->wDataSize); k++)
      COUTPUT(("%d,", dlginit_data->bData[j+k]));
    COUTPUT(("\n"));
  }
  COUTPUT(("};\n"));
  COUTPUT(("\n"));

}

void print_toolbar_res(int i, NAMEINFO *info)
{
  TOOLBAR_NODE *toolbar_node;
  TOOLBARDATA *toolbar_data;
  WORD item;

  toolbar_node = (TOOLBAR_NODE *)(info->rcsdata);
  toolbar_data = &(toolbar_node->data);

  COUTPUT(("static TOOLBARDATA u241_data_%d =\n", i));
  COUTPUT(("{\n"));
  COUTPUT(("\t%d,\t/* wVersion   */\n", toolbar_data->wVersion));
  COUTPUT(("\t%d,\t/* wWidth     */\n", toolbar_data->wWidth));
  COUTPUT(("\t%d,\t/* wHeight    */\n", toolbar_data->wHeight));
  COUTPUT(("\t%d,\t/* wItemCount */\n", toolbar_data->wItemCount));
  if (toolbar_data->wItemCount != 0)
    COUTPUT(("\t{\t/* aItems[]   */\n"));
  for (item = 0; item < toolbar_data->wItemCount; item++)
    COUTPUT(("\t\t%d,\n", toolbar_data->aItems[item]));
  if (toolbar_data->wItemCount != 0)
    COUTPUT(("\t}\n"));
  COUTPUT(("};\n"));
  COUTPUT(("\n"));

}

void null_func()
{
}

void print_name(int i, NAMEINFO *info, char *title)
{
  COUTPUT(( "\t{ (LPBYTE)%s_data_%d, ", title, i));
}

typedef void (*PRINTFUNC)(int, NAMEINFO *);
typedef void (*PRINTNAMEFUNC)(int, NAMEINFO *, char *);

struct print_def
{
  PRINTFUNC printFunc;
  char *title1;
  char *title2;
  PRINTNAMEFUNC printNameFunc;
} print_defs[] =
  {
    { print_cursor_res      , "cursor"    , "&cursor"   , print_name      },
    { print_bitmap_res      , "bitmap"    , "&bitmap"   , print_name      },
    { print_icon_res        , "icon"      , "&icon"     , print_name      },
    { print_menu_res        , "menu"      , "m"         , print_menu_name },
    { print_dialog_res      , "dialog"    , "&dialog"   , print_name      },
    { print_string_res      , "string"    , "string"    , print_name      },
    { print_fontdir_res     , "fontdir"   , "fontdir"   , print_name      },
    { print_font_res        , "font"      , "font"      , print_name      },
    { print_accel_res       , "accel"     , "accel"     , print_name      },
    { print_rcdata_res      , "rcdata"    , "rc"        , print_name      },
    { print_group_cursor_res, "cursor_dir", "cursor_dir", print_name      },
    { print_group_icon_res  , "icon_dir"  , "icon_dir"  , print_name      },
    { print_dlginit_res     , "u240"      , "&u240"     , print_name      },
    { print_toolbar_res     , "u241"      , "&u241"     , print_name      },
    { print_user_res        , userName    , userName    , print_name      }
  };

enum print_types {
	rtCursor, rtBitmap, rtIcon, rtMenu, rtDialog, rtString, rtFontDir,
	rtFont, rtAccel, rtRCData, rtGroupCursor, rtGroupIcon,
	rtDlgInit, rtToolBar,
	rtUser
};

void print_typed_resources(TYPEINFO *type_ptr, enum print_types pt)
{
  struct print_def *pd = &print_defs[pt];
  NAMEINFO_NODE *info;
  int i;

  info = (NAMEINFO_NODE *)(type_ptr->rcsinfo);
  for (i = 0; i < type_ptr->rcscount; i++, info = info->next)
    (pd->printFunc)(i, &(info->data));

  COUTPUT(("static NAMEINFO %s_info[] = {\n", pd->title1));
  info = (NAMEINFO_NODE *)(type_ptr->rcsinfo);
  for (i = 0; i < type_ptr->rcscount; i++, info = info->next)
    {
      (pd->printNameFunc)(i, &(info->data), pd->title2);
      COUTPUT(( "0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, ",
		info->data.wType, 0 /* info->data.hGlobal */,
		0 /* info->data.rcsoffset */, info->data.rcslength, info->data.rcsflags,
		0 /* info->data.hRsrc */, 0 /* info->data.hObject */));
      print_num_or_str(info->data.rcsitemname);
      COUTPUT(( "},\n"));	
    }
  print_last_entry(9);
  COUTPUT(("};\n\n"));
}

void print_func(TYPEINFO *type_ptr)
{
  switch((DWORD)type_ptr->rcstypename)
    {
    case (DWORD)RT_CURSOR:       print_typed_resources(type_ptr, rtCursor); break;
    case (DWORD)RT_BITMAP:       print_typed_resources(type_ptr, rtBitmap); break;
    case (DWORD)RT_ICON:         print_typed_resources(type_ptr, rtIcon); break;
    case (DWORD)RT_MENU:         print_typed_resources(type_ptr, rtMenu); break;
    case (DWORD)RT_DIALOG:       print_typed_resources(type_ptr, rtDialog); break;
    case (DWORD)RT_STRING:       print_typed_resources(type_ptr, rtString); break;
    case (DWORD)RT_FONTDIR:      print_typed_resources(type_ptr, rtFontDir); break;
    case (DWORD)RT_FONT:         print_typed_resources(type_ptr, rtFont); break;
    case (DWORD)RT_ACCELERATOR:  print_typed_resources(type_ptr, rtAccel); break;
    case (DWORD)RT_RCDATA:       print_typed_resources(type_ptr, rtRCData); break;
    case (DWORD)RT_GROUP_CURSOR: print_typed_resources(type_ptr, rtGroupCursor); break;
    case (DWORD)RT_GROUP_ICON:   print_typed_resources(type_ptr, rtGroupIcon); break;
    case (DWORD)RT_DLGINIT:      print_typed_resources(type_ptr, rtDlgInit); break;
    case (DWORD)RT_TOOLBAR:      print_typed_resources(type_ptr, rtToolBar); break;
    default:
      if (HIWORD(type_ptr->rcstypename) == 0)
	sprintf(userName, "u%d", (int)(type_ptr->rcstypename));
      else
	strcpy(userName, type_ptr->rcstypename);
      print_typed_resources(type_ptr, rtUser);
      break;
    }
}

void print_resources(void)
{
  TYPEINFO *type_ptr;
  int i;

  COUTPUT(("/* TWIN Resource Compiler Version " RC_VERSION " */\n"));
  COUTPUT(("\n"));
  COUTPUT(("#define NOAPIPROTO\n"));
  COUTPUT(("\n"));
  COUTPUT(("#include \"windows.h\"\n"));
  COUTPUT(("#include \"Resources.h\"\n"));
  COUTPUT(("\n"));
  for(i = 0; i < MAX_TYPES; i++)
  {
    type_ptr = resource_table + i;
    if (type_ptr->rcscount > 0 )
      print_func(type_ptr);
  }
  
  parse_module();      /* Load the Module Definition file */

  COUTPUT(("%sTYPEINFO hsmt_resource_%s[] = {\n",
      (strlen(tbl_name))?"":"static ",
      (strlen(tbl_name))?tbl_name:"table"));

  for (i = 0; i < MAX_TYPES; i++)
    {
      type_ptr = resource_table + i;
      if (type_ptr->rcscount > 0 )
	{
	  if (HIWORD((DWORD)type_ptr->rcstypename) == 0)
	    COUTPUT(( "\t{ (char *) %d, ", LOWORD((DWORD)type_ptr->rcstypename)));
	  else
	    COUTPUT(( "\t{ \"%s\", ", type_ptr->rcstypename));
	  COUTPUT(( " %d, ", type_ptr->rcscount));
	  if ((HIWORD((DWORD)type_ptr->rcstypename) == 0) &&
	      (LOWORD((DWORD)type_ptr->rcstypename) < 16))
	    COUTPUT(( " %s },\n", info_name_tbl[LOWORD((DWORD)type_ptr->rcstypename) - 1]));
	  else
	    if (HIWORD(type_ptr->rcstypename) == 0)
	      COUTPUT(( "u%d_info},\n", (int)type_ptr->rcstypename));
	    else
	      COUTPUT(( "%s_info},\n", type_ptr->rcstypename));
	  
	}
    }
  print_last_entry(3);
  COUTPUT(("};\n\n"));
  print_module();
}

