/*    
	resources.c	1.18
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
#include <stdlib.h>
#include <string.h>
#include "windows.h"
#include "Resources.h"
#include "BinTypes.h"

#include "rc.h"
#include "buffer.h"
#include "restypes.h"
#include "resfunc.h"

#define	TYPE_OFFSET	2
#define	CURSOR_DIR_OFFSET 6
#define	ICON_DIR_OFFSET 6
#define	DIR_ENTRY_SIZE	16
#define	BMP_FILE_HEADER 14
#define	PC_INT	short
#define	FLAG_DEFAULT	0x0c30

void add_common_resource(resp r);

TYPEINFO *resource_table = NULL;
NAMEENTRY_NODE *name_table = NULL;

/* 
 * returns a pointer to the NAMEINFO structure given the resource type.
 * null if can not find the resource.
 */
NAMEINFO_NODE *
get_nameinfo(char *type_name)
{
  TYPEINFO *type_ptr;
  int	i;
 
  for(i = 0; i < MAX_TYPES; i++)
    {
      type_ptr = resource_table + i;
      if (type_ptr->rcscount > 0 )
	if ((HIWORD(type_name) == 0) && (HIWORD(type_ptr->rcstypename) == 0))
	  {
	    if ((int) type_ptr->rcstypename == (int)type_name) 
	      return ((NAMEINFO_NODE *)type_ptr->rcsinfo);
	  }
	else if (HIWORD(type_name) && HIWORD(type_ptr->rcstypename))
	  if (!strcmp(type_name, type_ptr->rcstypename))
	    return ((NAMEINFO_NODE *)type_ptr->rcsinfo);
    }
  return (NAMEINFO_NODE *)NULL;
	
}

void add_resource(const char *res_name, int flags, resp r)
{
  if (!r)
    return;
  r->name = res_name;
  r->flags = flags;
  add_common_resource(r);
}

strp resource_type_name(const char *type)
{
  strp buf = str_alloc(256);
  switch ((unsigned long)type)
    {
    case (unsigned long)RT_BITMAP:
      strcpy(buf, "Bitmap");
      break;
    case (unsigned long)RT_GROUP_CURSOR:
      strcpy(buf, "Cursor");
      break;
    case (unsigned long)RT_CURSOR:
      strcpy(buf, "Cursor image");
      break;
    case (unsigned long)RT_GROUP_ICON:
      strcpy(buf, "Icon");
      break;
    case (unsigned long)RT_ICON:
      strcpy(buf, "Icon image");
      break;
    case (unsigned long)RT_MENU:
      strcpy(buf, "Menu");
      break;
    case (unsigned long)RT_DIALOG:
      strcpy(buf, "Dialog");
      break;
    case (unsigned long)RT_DLGINIT:
      strcpy(buf, "DlgInit");
      break;
    case (unsigned long)RT_TOOLBAR:
      strcpy(buf, "Toolbar");
      break;
    case (unsigned long)RT_ACCELERATOR:
      strcpy(buf, "Accelerator table");
      break;
    case (unsigned long)RT_STRING:
      strcpy(buf, "String table");
      break;
    case (unsigned long)RT_RCDATA:
      strcpy(buf, "RCDATA");
      break;
    default:
      if (HIWORD(type))
	strcpy(buf, type);
      else
	sprintf(buf, "User-defined type %d", (int)type);
    }
  return buf;
}

void add_common_resource(resp r)
{
  char outTemp[256];
  strp outType;
  NAMEINFO_NODE   *new_nameinfo;

  if (!r || !r->b || !r->b->buffer)
    return; /* Refuse to add a resource without data */
   
  ALLOC(new_nameinfo, NAMEINFO_NODE);
  new_nameinfo->next = NULL;
  new_nameinfo->data.rcslength = r->b->len;
  new_nameinfo->data.rcsdata = (LPBYTE)buff_ind(r->b);
  r->b = NULL;
  new_nameinfo->data.rcsflags = r->flags;
  if (!HIWORD(r->name))
    {
      if (LOWORD(r->name) > 0x7fff)
	FATAL(("Resource identifier %d is too large", (int)(LOWORD(r->name))));
      new_nameinfo->data.rcsitemname = (char *)(LOWORD(r->name) | (r->type == RT_STRING ? 0x0000 : 0x8000));
    }
  else
    new_nameinfo->data.rcsitemname = (char *)(r->name);
  if (HIWORD(r->type))
    new_nameinfo->data.wType = (WORD)(RT_USER_DEF);
  else
    new_nameinfo->data.wType = LOWORD(r->type);
  incr_res_count((char *)r->type, new_nameinfo);
  if (!HIWORD(r->name))
    sprintf(outTemp, "%d", (int)r->name);
  else
    sprintf(outTemp, "%s", r->name);
  outType = resource_type_name(r->type);
  VPRINT(("Added resource: %s %s", outType, outTemp));
  str_free(outType);
  res_free(r);
}

resp make_bitmap_resource(buffp b)
{
  WORD	rgb_count;
  DWORD	image_size;
  BITMAPINFOHEADER   *bmp_info_header;
  BITMAP_NODE  *bmp_image;
  resp r;
  RGBQUAD	*rgb_quad;
  char	*bmp_bits;
  
  r = res_new();
  r->type = (strp)RT_BITMAP;
  r->flags = FLAG_DEFAULT;

  if (*(char *)b->buffer != 'B' ||
      *(((char *)b->buffer) + 1) != 'M')
    FATAL(("Invalid bitmap resource"));
  
  ALLOC(bmp_info_header, BITMAPINFOHEADER);
  get_bmp_info_header(bmp_info_header, (BYTE *)b->buffer + BMP_FILE_HEADER);
  
  rgb_count = bit_to_rgb(bmp_info_header->biBitCount);
  CALLOC(rgb_quad, rgb_count, RGBQUAD);
  memcpy(rgb_quad, (BYTE *)b->buffer + BMP_FILE_HEADER + bmp_info_header->biSize,
	 rgb_count * sizeof(RGBQUAD));
  
  image_size = bmp_info_header->biSizeImage;
  NALLOC(bmp_bits, image_size, char *);
  memcpy(bmp_bits, (BYTE *)b->buffer + BMP_FILE_HEADER +
	 bmp_info_header->biSize + rgb_count * sizeof(RGBQUAD),
	 image_size);
  
  BUFF_STRUCT(r->b, bmp_image, BITMAP_NODE);

  bmp_image->data.bmp_hdr = bmp_info_header;
  bmp_image->data.rgb_quad = rgb_quad;
  bmp_image->data.bitmap_bits = (BYTE*)bmp_bits;
  
  buff_free(b);

  return r;
}

resp make_cursor_resource(buffp b)
{
  WORD i, type, count, HotX, HotY;
  CURSORDIR_NODE *cur_entry, *cursor_entry_table;
  CURSOR_NODE *cur_image;
  DWORD	image_offset, image_size;
  BYTE *temp_offset;
  BITMAPINFOHEADER  *bmp_info_header;
  RGBQUAD *rgb_quad;
  char *XOR_bits, *AND_bits;
  WORD mask_bytes;
  static unsigned IdOrdinal = 1;
  resp r, rCursor;

  type = get_word((BYTE *)b->buffer + TYPE_OFFSET);
  count = get_word((BYTE *)b->buffer + TYPE_OFFSET + 2);
  if (type != 2)
    FATAL(("Invalid cursor resource, type=%d", type));

  r = res_new();
  r->type = (char *)RT_GROUP_CURSOR;
  r->flags = FLAG_DEFAULT;

  BUFF_STRUCT(r->b, cursor_entry_table, CURSORDIR_NODE);
  cur_entry = cursor_entry_table;
  cur_entry->next = NULL;
  i = 0;

  while (1) {
    temp_offset = (BYTE *)b->buffer + CURSOR_DIR_OFFSET + i * DIR_ENTRY_SIZE;
    cur_entry->data.wWidth = temp_offset[0];
    cur_entry->data.wHeight = temp_offset[1];
    cur_entry->data.wOrdinalNumber = IdOrdinal;
    cur_entry->data.wPlanes = 0;
    cur_entry->data.wBitsPerPel = 0;
    cur_entry->data.dwBytesInRes = 0;
    HotX = get_word((BYTE *)&temp_offset[4]);
    HotY = get_word((BYTE *)&temp_offset[6]);
    image_size = get_dword((BYTE *)&temp_offset[8]);
    image_offset = get_dword((BYTE *)&temp_offset[12]);

    temp_offset = (BYTE *)b->buffer + image_offset;

    ALLOC(bmp_info_header, BITMAPINFOHEADER);
    get_bmp_info_header(bmp_info_header, temp_offset);

    temp_offset += bmp_info_header->biSize;

    NALLOC(rgb_quad, sizeof(RGBQUAD) * 2, RGBQUAD *);
    memcpy((char *)rgb_quad, (char *)temp_offset, sizeof(RGBQUAD) * 2);

    temp_offset += sizeof(RGBQUAD) * 2;

    mask_bytes =  (bmp_info_header->biWidth + 31)/32; 
    mask_bytes = mask_bytes << 2;
    mask_bytes = (int)(mask_bytes * bmp_info_header->biHeight)/2;

    NALLOC(AND_bits, mask_bytes, char *);
    NALLOC(XOR_bits, mask_bytes, char *);

    memcpy(XOR_bits, (char *)temp_offset, mask_bytes);
    memcpy(AND_bits, (char *)temp_offset + mask_bytes, mask_bytes);

    rCursor = res_new();
    BUFF_STRUCT(rCursor->b, cur_image, CURSOR_NODE);
    cur_image->data.wHotSpotX = HotX;
    cur_image->data.wHotSpotY = HotY;
    cur_image->data.bmp_hdr = bmp_info_header;
    cur_image->data.rgb_quad = rgb_quad;
    cur_image->data.xor_mask = (BYTE*)XOR_bits;
    cur_image->data.and_mask = (BYTE*)AND_bits;

    rCursor->type = RT_CURSOR;
    rCursor->name = (const char *)(IdOrdinal);
    /* Flags? */
    add_common_resource(rCursor);
    
    IdOrdinal++;
    if (count == ++i)
      break;
    else {
      ALLOC(cur_entry->next, CURSORDIR_NODE);
      cur_entry = cur_entry->next;
      cur_entry->next = (CURSORDIR_NODE *)NULL;
    }	
  }
  
  buff_free(b);

  return r;
}

resp make_icon_resource(buffp b)
{
  BYTE *temp;
  WORD	i, type, count, rgb_count;
  ICONDIR_NODE *cur_entry, *icon_entry_table;
  ICON_NODE *cur_image;
  DWORD	image_offset, image_size;
  BYTE	*temp_offset;
  BITMAPINFOHEADER  *bmp_info_header;
  RGBQUAD	*rgb_quad;
  char	*XOR_bits, *AND_bits;
  WORD	mask_bytes;
  static unsigned IdOrdinal = 1;
  resp r, rIcon;
  
  type = get_word((BYTE *)b->buffer + TYPE_OFFSET);
  count = get_word((BYTE *)b->buffer + TYPE_OFFSET + 2);
  if (type != 1)
    FATAL(("Invalid icon resource, type=%d", type));
  
  r = res_new();
  r->type = (char *)RT_GROUP_ICON;
  r->flags = FLAG_DEFAULT;

  r->b = buff_alloc(count * sizeof(ICONDIR_NODE));
  r->b->len = r->b->alloc;
  icon_entry_table = (ICONDIR_NODE *)r->b->buffer;

  cur_entry = icon_entry_table;
  cur_entry->next = (ICONDIR_NODE *)NULL;
  i = 0;
  while (1)
    {
      temp = (BYTE *)b->buffer + ICON_DIR_OFFSET + i * DIR_ENTRY_SIZE;
      cur_entry->data.bWidth = temp[0];
      cur_entry->data.bHeight = temp[1];
      cur_entry->data.wOrdinalNumber = IdOrdinal;
      cur_entry->data.bColorCount = temp[2];
      cur_entry->data.wPlanes = get_word(&temp[4]);
      cur_entry->data.wBitsPerPel = 0; /* Unused by library */
      cur_entry->data.dwBytesInRes = 0; /* Unused by library */
      image_size = get_dword(&temp[8]);
      image_offset = get_dword(&temp[12]);

      temp_offset = (BYTE *)b->buffer + image_offset;

      ALLOC(bmp_info_header, BITMAPINFOHEADER);
      get_bmp_info_header(bmp_info_header, temp_offset);
      temp_offset += bmp_info_header->biSize;

      rgb_count = bit_to_rgb(bmp_info_header->biBitCount);
      CALLOC(rgb_quad, rgb_count, RGBQUAD);
      memcpy((char *)rgb_quad, (char *)temp_offset,sizeof(RGBQUAD)*rgb_count);
      temp_offset += sizeof(RGBQUAD) * rgb_count;
      
      mask_bytes =  (int)(bmp_info_header->biWidth *
			  bmp_info_header->biHeight * bmp_info_header->biBitCount) / 16;
      
      NALLOC(XOR_bits, mask_bytes, char *);
      memcpy(XOR_bits, (char *)temp_offset, mask_bytes);
      temp_offset += mask_bytes;

      mask_bytes /=  bmp_info_header->biBitCount;
      /* AND mask is always MonoChrome */

      NALLOC(AND_bits, mask_bytes, char *);
      memcpy(AND_bits, (char *)temp_offset, mask_bytes);

      rIcon = res_new();
      BUFF_STRUCT(rIcon->b, cur_image, ICON_NODE);
      cur_image->data.icon_bmp_hdr = bmp_info_header;
      cur_image->data.icon_rgb_quad = rgb_quad;
      cur_image->data.icon_xor_mask = (BYTE*)XOR_bits;
      cur_image->data.icon_and_mask = (BYTE*)AND_bits;
      
      rIcon->type = RT_ICON;
      rIcon->name = (const char *)(IdOrdinal);
      /* Flags? */
      add_common_resource(rIcon);
      
      IdOrdinal++;
      if (count == ++i)
	break;
      else {
	ALLOC(cur_entry->next, ICONDIR_NODE);
	cur_entry = cur_entry->next;
	cur_entry->next = (ICONDIR_NODE *)NULL;
      }
    }

  buff_free(b);

  return r;
}

resp make_menu_resource(buffp b)
{
  resp r = res_new();
  r->type = (char *)RT_MENU;
  r->flags = FLAG_DEFAULT;
  r->b = b;
  r->name = NULL;
  return r;
}

buffp append_menu_node(buffp b, buffp append)
{
  MENU_NODE *pNode;
  if (!b)
    return append;
  pNode = (MENU_NODE *)b->buffer;
  if (!pNode)
    return append;
  while (pNode->next)
    pNode = pNode->next;
  pNode->next = (MENU_NODE *)buff_ind(append);
  return b;
}

buffp set_menu_last(buffp b)
{
  MENU_NODE *pNode;
  if (!b)
    return b;
  pNode = (MENU_NODE *)b->buffer;
  if (!pNode)
    {
      buff_free(b);
      return NULL;
    }
  while (pNode->next)
    pNode = pNode->next;
  pNode->data.menuflag |= MF_END;
  return b;
}

buffp create_menu_popup(strp name, buffp popup, int flags)
{
  MENU_NODE *pNode;
  buffp b;

  b = buff_alloc(sizeof(MENU_NODE));
  pNode = (MENU_NODE *)b->buffer;
  b->len = sizeof(MENU_NODE);

  pNode->data.menuflag = flags | MF_POPUP;
  pNode->data.menuname = name;
  pNode->data.menuid = 0;
  pNode->data.menupopup = NULL;
  pNode->next = NULL;
  pNode->child = (MENU_NODE *)buff_ind(popup);

  return b;
}

buffp create_menu_item(char *name, int id, int flags)
{
  MENU_NODE *pNode;
  buffp b;

  BUFF_STRUCT(b, pNode, MENU_NODE);
  pNode->data.menuflag = flags;
  pNode->data.menuname = name;
  pNode->data.menuid = id;
  pNode->data.menupopup = NULL;
  pNode->next = NULL;
  pNode->child = NULL;

  return b;
}

buffp create_menu_separator(void)
{
  MENU_NODE *pNode;
  buffp b;

  BUFF_STRUCT(b, pNode, MENU_NODE);
  pNode->data.menuflag = MF_SEPARATOR;
  pNode->data.menuname = NULL;
  pNode->data.menuid = 0;
  pNode->data.menupopup = NULL;
  pNode->next = NULL;
  pNode->child = NULL;
  return b;
}

unsigned menu_checked(void) { return MF_CHECKED; }
unsigned menu_grayed(void) { return MF_GRAYED; }
unsigned menu_inactive(void) { return MF_DISABLED; }
unsigned menu_help(void) { return MF_HELP; }

resp make_dialog_resource(buffp b)
{
  resp r = res_new();
  r->type = (char *)RT_DIALOG;
  r->flags = FLAG_DEFAULT;
  r->b = b;
  r->name = NULL;
  return r;
}

buffp make_dialog(struct dimvals dims)
{
  DIALOG_NODE *dialog_data;
  buffp b;

  BUFF_STRUCT(b, dialog_data, DIALOG_NODE);
  dialog_data->data.lStyle = WS_POPUPWINDOW;
  dialog_data->data.x = dims.corner.x;
  dialog_data->data.y = dims.corner.y;
  dialog_data->data.cx = dims.extent.x;
  dialog_data->data.cy = dims.extent.y;
  dialog_data->data.szMenuName = NULL;
  dialog_data->data.szClassName = NULL;
  dialog_data->data.szCaption = NULL;
  dialog_data->data.wPointSize = 0;
  dialog_data->data.szFaceName = NULL;
  dialog_data->data.bNumberOfItems = 0;
  dialog_data->controlinfo = NULL;

  return b;
}

buffp set_dialog_style(buffp b, int style)
{
  ((DIALOG_NODE *)(b->buffer))->data.lStyle = style;
  return b;
}

buffp set_dialog_caption(buffp b, char *caption)
{
  ((DIALOG_NODE *)(b->buffer))->data.szCaption = caption;
  return b;
}

buffp set_dialog_class(buffp b, char *dlgclass)
{
  ((DIALOG_NODE *)(b->buffer))->data.szClassName = dlgclass;
  return b;
}

buffp set_dialog_font(buffp b, char *fontname, int fontsize)
{
  ((DIALOG_NODE *)(b->buffer))->data.szFaceName = fontname;
  ((DIALOG_NODE *)(b->buffer))->data.wPointSize = fontsize;
  ((DIALOG_NODE *)(b->buffer))->data.lStyle |= DS_SETFONT;
  return b;
}

buffp set_dialog_menu(buffp b, const char *menuid)
{
  ((DIALOG_NODE *)(b->buffer))->data.szMenuName = (char *)menuid;
  return b;
}

buffp set_dialog_controls(buffp b, buffp controls)
{
  DIALOG_NODE *dialog_data = (DIALOG_NODE *)b->buffer;
  CONTROL_NODE *iter = (CONTROL_NODE *)buff_ind(controls);

  dialog_data->data.bNumberOfItems = 0;
  dialog_data->controlinfo = iter;
  while (iter)
    {
      dialog_data->data.bNumberOfItems++;
      iter = iter->next;
    }
  return b;
}

buffp append_control(buffp b, buffp append)
{
  CONTROL_NODE *pNode;
  if (!b)
    return append;
  pNode = (CONTROL_NODE *)b->buffer;
  if (!pNode)
    return append;
  while (pNode->next)
    pNode = pNode->next;
  pNode->next = (CONTROL_NODE *)buff_ind(append);
  return b;
}

#define S(x) strp s = str_dup(x)

buffp create_ltext_control(char *name, int id, struct dimvals dims, struct maskvals style)
{
  S("STATIC");
  return make_control(name, id, s, style, dims, WS_CHILD | WS_VISIBLE | WS_GROUP | SS_LEFT);
}

buffp create_rtext_control(char *name, int id, struct dimvals dims, struct maskvals style)
{
  S("STATIC");
  return make_control(name, id, s, style, dims, WS_CHILD | WS_VISIBLE | WS_GROUP | SS_RIGHT);
}

buffp create_ctext_control(char *name, int id, struct dimvals dims, struct maskvals style)
{
  S("STATIC");
  return make_control(name, id, s, style, dims, WS_CHILD | WS_VISIBLE | WS_GROUP | SS_CENTER);
}

buffp create_icon_control(char *name, int id, struct dimvals dims, struct maskvals style)
{
  S("STATIC");
  return make_control(name, id, s, style, dims, WS_VISIBLE | WS_CHILD | WS_GROUP | SS_ICON);
}

buffp create_edit_control(int id, struct dimvals dims, struct maskvals style)
{
  S("EDIT");
  return make_control(NULL, id, s, style, dims, WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP | WS_BORDER);
}

buffp create_listbox_control(int id, struct dimvals dims, struct maskvals style)
{
  S("LISTBOX");
  return make_control(NULL, id, s, style, dims, WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY);
}

buffp create_combobox_control(int id, struct dimvals dims, struct maskvals style)
{
  S("COMBOBOX");
  if (style.or & 0x3)
    style.and &= ~0x3;
  return make_control(NULL, id, s, style, dims, WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_SIMPLE);
}

buffp create_groupbox_control(char *name, int id, struct dimvals dims, struct maskvals style)
{
  S("BUTTON");
  return make_control(name, id, s, style, dims, WS_VISIBLE | WS_CHILD | BS_GROUPBOX);
}

buffp create_pushbutton_control(char *name, int id, struct dimvals dims, struct maskvals style)
{
  S("BUTTON");
  return make_control(name, id, s, style, dims, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON);
}

buffp create_defpushbutton_control(char *name, int id, struct dimvals dims, struct maskvals style)
{
  S("BUTTON");
  return make_control(name, id, s, style, dims, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON);
}

buffp create_checkbox_control(char *name, int id, struct dimvals dims, struct maskvals style)
{
  S("BUTTON");
  return make_control(name, id, s, style, dims, WS_VISIBLE | WS_CHILD | BS_CHECKBOX | WS_TABSTOP);
}

buffp create_radiobutton_control(char *name, int id, struct dimvals dims, struct maskvals style)
{
  S("BUTTON");
  return make_control(name, id, s, style, dims, WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON | WS_TABSTOP );
}

buffp create_autoradiobutton_control(char *name,int id, struct dimvals dims, struct maskvals style)
{
  S("BUTTON");
  return make_control(name, id, s, style, dims, WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_TABSTOP);
}

buffp create_scrollbar_control(int id, struct dimvals dims, struct maskvals style)
{
  S("SCROLLBAR");
  return make_control(NULL, id, s, style, dims, WS_VISIBLE | WS_CHILD);
}

buffp create_generic_control(char *name, int id, char *class, struct maskvals style, struct dimvals dims)
{
  return make_control(name, id, class, style, dims, WS_VISIBLE | WS_CHILD);
}

buffp make_control(char *name, int id, char *class, struct maskvals stylemask, struct dimvals dims, int style)
{
  buffp b;
  CONTROL_NODE *pNode;

  if (!istrcmp(class, "button"))
      if (stylemask.or & 0xF)
	stylemask.and &= ~(0xF);

  BUFF_STRUCT(b, pNode, CONTROL_NODE);
  pNode->data.lStyle = (style & stylemask.and) | stylemask.or;
  pNode->data.x = dims.corner.x;
  pNode->data.y = dims.corner.y;
  pNode->data.cx = dims.extent.x;
  pNode->data.cy = dims.extent.y;
  pNode->data.wID = id;
  pNode->data.szText = name;
  pNode->data.szClass = class;
  pNode->next = NULL;

  return b;
}

resp make_dlginit_resource(buffp b)
{
  resp r = res_new();
  r->type = (char *)RT_DLGINIT;
  r->flags = FLAG_DEFAULT;
  r->b = b;
  r->name = NULL;
  return r;
}

buffp make_dlginit()
{
  DLGINIT_NODE *dlginit_data;
  buffp b;

  BUFF_STRUCT(b, dlginit_data, DLGINIT_NODE);
  dlginit_data->data.wDataSize = 0;
  return b;

}

buffp add_dlginit_item(buffp b, BYTE bItem)
{
  DLGINIT_NODE *dlginit_data = (DLGINIT_NODE *)((b)->buffer);

  if (dlginit_data->data.wDataSize >=
	sizeof(dlginit_data->data.bData)/sizeof(dlginit_data->data.bData[0]))
    CERROR(("Too many dlginit items.\n"));

  dlginit_data->data.bData[dlginit_data->data.wDataSize++] = bItem;

  return b;

}

resp make_toolbar_resource(buffp b)
{
  resp r = res_new();
  r->type = (char *)RT_TOOLBAR;
  r->flags = FLAG_DEFAULT;
  r->b = b;
  r->name = NULL;
  return r;
}

buffp make_toolbar(WORD wVersion, WORD wWidth, WORD wHeight)
{
  TOOLBAR_NODE *toolbar_data;
  buffp b;

  BUFF_STRUCT(b, toolbar_data, TOOLBAR_NODE);
  toolbar_data->data.wVersion = wVersion;
  toolbar_data->data.wWidth = wWidth;
  toolbar_data->data.wHeight = wHeight;
  toolbar_data->data.wItemCount = 0;
  return b;

}

buffp set_toolbar_dims(buffp b, WORD wWidth, WORD wHeight)
{
  TOOLBAR_NODE *toolbar_data = (TOOLBAR_NODE *)((b)->buffer);

  toolbar_data->data.wWidth = wWidth;
  toolbar_data->data.wHeight = wHeight;
  return b;

}

buffp add_toolbar_item(buffp b, WORD wItem)
{
  TOOLBAR_NODE *toolbar_data = (TOOLBAR_NODE *)((b)->buffer);

  if (toolbar_data->data.wItemCount >= 32)
    CERROR(("Too many toolbar items.\n"));

  toolbar_data->data.aItems[toolbar_data->data.wItemCount++] = wItem;

  return b;

}

resp make_accelerator_resource(buffp b)
{
  resp r = res_new();
  r->type = (char *)RT_ACCELERATOR;
  r->flags = FLAG_DEFAULT;
  r->b = b;
  r->name = NULL;
  return r;
}

buffp append_accel(buffp b, buffp append)
{
  ACCEL_NODE *pNode;
  if (!b)
    return append;
  pNode = (ACCEL_NODE *)b->buffer;
  while (pNode->next)
    pNode = pNode->next;
  pNode->next = (ACCEL_NODE *)buff_ind(append);
  return b;
}

buffp make_accel_s(char *accel, int id, int flags)
{
  buffp b;
  char *p = accel;
  
  if (*p == '^')
    {
      flags |= 0x08; /* Why? */
      p++;
    }
  b = make_accel_i(*p, id, flags);
  str_free(accel);
  return b;
}

buffp make_accel_i(int accel, int id, int flags)
{
  ACCEL_NODE *pNode;
  buffp b;

  BUFF_STRUCT(b, pNode, ACCEL_NODE);
  pNode->next = NULL;
  pNode->data.fFlags = flags;
  pNode->data.wId = id;
  pNode->data.wEvent = accel;

  return b;
}

/* Here we emulate the (probably wrong) behavior of the old rc...
 *
 * Rather than let each string segment have its own flags, we force
 * one set of flags on all string resources.  In particular, this will
 * be the _last_ set of flags specified for any stringtable resource.
 */

void set_string_flags(int flags)
{
  NAMEINFO_NODE *temp_info;

  temp_info = get_nameinfo((char *)RT_STRING);
  while(temp_info)
    {
      temp_info->data.rcsflags = flags;
      temp_info = temp_info->next;
    }
}

STRING_DATA *find_string_segment(int id)
{
  int segnum = id / 16 + 1;
  int i;
  resp r;
  NAMEINFO_NODE *temp_info;
  STRING_DATA	*new_data;

  temp_info = get_nameinfo((char *)RT_STRING);
  while(temp_info)
      if ((int)(temp_info->data.rcsitemname) == segnum)
	return (STRING_DATA *)(temp_info->data.rcsdata);
      else
	temp_info = temp_info->next;

  r = res_new();
  r->type = RT_STRING;
  r->name = (const char *)segnum;
  BUFF_STRUCT(r->b, new_data, STRING_DATA);
  for (i=0; i < 16; i++)
    (*new_data)[i] = (char *) NULL;
  add_common_resource(r);
  return new_data;
}

void add_table_string(int id, char *string)
{
  STRING_DATA *pData = find_string_segment(id);
  (*pData)[id % 16] = string;
}

resp type_resource_rcdata(resp r)
{
  r->type = (char *)RT_RCDATA;
  return r;
}

resp type_resource(resp r, const char *type)
{
  r->type = (char *)type;
  return r;
}

resp make_binary_resource(buffp b)
{
  resp r = res_new();
  r->type = NULL;
  r->flags = FLAG_DEFAULT;
  r->b = b;
  r->name = NULL;
  return r;
}

int alignval(char *str)
{
  switch (str[0])
    {
    case '1':
      return 1;

    case '2':
    case 's':
    case 'S':
      return 2;

    case '4':
    case 'l':
    case 'L':
      return 4;
    }

  CERROR(("Unsupported alignment value %s", str));
  return defbinflags.align;
}

buffp pack_rcdata(struct binflagvals f, struct userresbuffer *urbStart)
{
  unsigned len, length;
  buffp b;
  struct userresbuffer *urb;
  BYTE *bufp;
  
  length = 0;
  len    = 0;
  for (urb = urbStart; urb; urb = urb->next)
    switch (urb->t)
      {
      case string:
	length += len = strlen(urb->b.s);
	if (len % f.align)
	  length += f.align - (len % f.align);
	break;

      case shortint:
	length += len = 2;
	if (len % f.align)
	  length += f.align - (len % f.align);
	break;

      case longint:
	length += len = 4;
	if (len % f.align)
	  length += f.align - (len % f.align);
	break;

      case binary:
	length += len = urb->b.b->len;
	if (len % f.align)
	  length += f.align - (len % f.align);

      default:
	FATAL(("Unrecognized user data internal type %d", urb->t));
      }

  b = buff_alloc(length);
  b->len = length;
  bufp = (BYTE *)(b->buffer);

  for (urb = urbStart; urb; (urb = urb->next), (bufp += len))
    switch (urb->t)
      {
      case string:
	strcpy((char *)bufp, urb->b.s);
	len = strlen(urb->b.s);
	if (len % f.align)
	  len += f.align - (len % f.align);
	break;

      case shortint:
	if (f.endian == little)
	  {
	    bufp[0] = urb->b.i & 0xff;
	    bufp[1] = (urb->b.i >> 8) & 0xff;
	  }
	else if (f.endian == big)
	  {
	    bufp[1] = urb->b.i & 0xff;
	    bufp[0] = (urb->b.i >> 8) & 0xff;
	  }
	else
	  FATAL(("Unrecognized internal endian type %d", f.endian));
	len = 2;
	if (len % f.align)
	  len += f.align - (len % f.align);
	break;

      case longint:
	if (f.endian == little)
	  {
	    bufp[0] = urb->b.i & 0xff;
	    bufp[1] = (urb->b.i >> 8) & 0xff;
	    bufp[2] = (urb->b.i >> 16) & 0xff;
	    bufp[3] = (urb->b.i >> 24) & 0xff;
	  }
	else if (f.endian == big)
	  {
	    bufp[3] = urb->b.i & 0xff;
	    bufp[2] = (urb->b.i >> 8) & 0xff;
	    bufp[1] = (urb->b.i >> 16) & 0xff;
	    bufp[0] = (urb->b.i >> 24) & 0xff;
	  }
	else
	  FATAL(("Unrecognized internal endian type %d", f.endian));
	len = 4;
	if (len % f.align)
	  len += f.align - (len % f.align);
	break;

      case binary:
	memcpy(bufp, urb->b.b->buffer, urb->b.b->len);
	len = urb->b.b->len;
	if (len % f.align)
	  len += f.align - (len % f.align);
	break;
      }

  return b;
}

struct userresbuffer *urb_string(char *str)
{
  struct userresbuffer *urb;
  ALLOC(urb, struct userresbuffer);
  urb->next = NULL;
  urb->t = string;
  urb->b.s = str;
  return urb;
}

struct userresbuffer *urb_short(unsigned long i)
{
  struct userresbuffer *urb;
  ALLOC(urb, struct userresbuffer);
  urb->next = NULL;
  urb->t = shortint;
  urb->b.i = i;
  return urb;
}

struct userresbuffer *urb_long(unsigned long i)
{
  struct userresbuffer *urb;
  ALLOC(urb, struct userresbuffer);
  urb->next = NULL;
  urb->t = longint;
  urb->b.i = i;
  return urb;
}

struct userresbuffer *urb_binary(buffp b)
{
  struct userresbuffer *urb;
  ALLOC(urb, struct userresbuffer);
  urb->next = NULL;
  urb->t = binary;
  urb->b.b = b;
  return urb;
}

struct userresbuffer *urb_cat(struct userresbuffer *urb1, struct userresbuffer *urb2)
{
  struct userresbuffer *urb;

  if (!urb1)
    return urb2;

  for (urb = urb1; urb->next; urb = urb->next);
  urb->next = urb2;
  return urb1;
}

char *makeintresource(int id)
{
  return MAKEINTRESOURCE(id);
}
