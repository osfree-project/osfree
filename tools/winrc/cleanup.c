/*    
	cleanup.c	1.1
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

#if defined(__WATCOMC__) && defined(_WIN32)
#include "basetsd.h"
#endif

#include "windows.h"
#include "Resources.h"
#include "rc.h"
#include "restypes.h"
#include "buffer.h"

void free_cursor_res(NAMEINFO *info)
{
  CURSOR_NODE *node = (CURSOR_NODE *)(info->rcsdata);
  CURSORIMAGE *image = &(node->data);
  free_mem(image->bmp_hdr);
  free_mem(image->rgb_quad);
  free_mem(image->xor_mask);
  free_mem(image->and_mask);
  free_mem(node);
}

void free_bitmap_res(NAMEINFO *info)
{
  BITMAP_NODE *node = (BITMAP_NODE *)(info->rcsdata);
  BITMAPIMAGE *image = &(node->data);
  free_mem(image->bmp_hdr);
  free_mem(image->rgb_quad);
  free_mem(image->bitmap_bits);
  free_mem(node);
}

void free_icon_res(NAMEINFO *info)
{
  ICON_NODE *node = (ICON_NODE *)(info->rcsdata);
  ICONIMAGE *image = &(node->data);
  free_mem(image->icon_bmp_hdr);
  free_mem(image->icon_rgb_quad);
  free_mem(image->icon_xor_mask);
  free_mem(image->icon_and_mask);
  free_mem(node);
}

void free_menu_list(MENU_NODE *node)
{
  MENU_NODE *next;
  while (node)
    {
      next = node->next;
      if (node->child)
	free_menu_list(node->child);
      str_free(node->data.menuname);
      free_mem(node);
      node = node->next;
    }
}

void free_menu_res(NAMEINFO *info)
{
  free_menu_list((MENU_NODE *)(info->rcsdata));
}

void free_control_list(CONTROL_NODE *node)
{
  CONTROL_NODE *next;
  while (node)
    {
      next = node->next;
      str_free(node->data.szText);
      str_free(node->data.szClass);
      free_mem(node);
      node = next;
    }
}

void free_dialog_res(NAMEINFO *info)
{
  DIALOG_NODE *node = (DIALOG_NODE *)(info->rcsdata);
  free_control_list(node->controlinfo);
  str_free(node->data.szMenuName);
  str_free(node->data.szClassName);
  str_free(node->data.szCaption);
  str_free(node->data.szFaceName);
  free_mem(node);
}

void free_string_res(NAMEINFO *info)
{
  STRING_DATA *data = (STRING_DATA *)(info->rcsdata);
  int i;
  for (i = 0; i < 16; i++)
    str_free((*data)[i]);
  free_mem(data);
}

void free_accel_res(NAMEINFO *info)
{
  ACCEL_NODE *node, *next;
  node = (ACCEL_NODE *)(info->rcsdata);
  while (node)
    {
      next = node->next;
      free_mem(node);
      node = next;
    }
}

void free_group_cursor_res(NAMEINFO *info)
{
  CURSORDIR_NODE *next, *node = (CURSORDIR_NODE *)(info->rcsdata);
  while (node)
    {
      next = node->next;
      free_mem(node);
      node = next;
    }
}

void free_group_icon_res(NAMEINFO *info)
{
  ICONDIR_NODE *next, *node = (ICONDIR_NODE *)(info->rcsdata);
  while (node)
    {
      next = node->next;
      free_mem(node);
      node = next;
    }
}

void free_binary_res(NAMEINFO *info)
{
  free_mem(info->rcsdata);
}

void free_typed_resources(TYPEINFO *type_ptr, void (*freefn)(NAMEINFO *))
{
  NAMEINFO_NODE *info, *next;
  int i;

  info = (NAMEINFO_NODE *)(type_ptr->rcsinfo);
  for (i = 0; i < type_ptr->rcscount; i++, info = next)
    {
      (freefn)(&(info->data));
      next = info->next;
      free_mem(info);
    }
  type_ptr->rcsinfo = NULL;
  type_ptr->rcscount = 0;
}

void free_res_type(TYPEINFO *type_ptr)
{
  switch((DWORD)type_ptr->rcstypename)
    {
    case (DWORD)RT_CURSOR:       free_typed_resources(type_ptr, free_cursor_res);       break;
    case (DWORD)RT_BITMAP:       free_typed_resources(type_ptr, free_bitmap_res);       break;
    case (DWORD)RT_ICON:         free_typed_resources(type_ptr, free_icon_res  );       break;
    case (DWORD)RT_MENU:         free_typed_resources(type_ptr, free_menu_res  );       break;
    case (DWORD)RT_DIALOG:       free_typed_resources(type_ptr, free_dialog_res);       break;
    case (DWORD)RT_STRING:       free_typed_resources(type_ptr, free_string_res);       break;
    case (DWORD)RT_FONTDIR:                                                             break;
    case (DWORD)RT_FONT:                                                                break;
    case (DWORD)RT_ACCELERATOR:  free_typed_resources(type_ptr, free_accel_res);        break;
    case (DWORD)RT_GROUP_CURSOR: free_typed_resources(type_ptr, free_group_cursor_res); break;
    case (DWORD)RT_GROUP_ICON:   free_typed_resources(type_ptr, free_group_icon_res);   break;
    case (DWORD)RT_RCDATA:       free_typed_resources(type_ptr, free_binary_res);       break;
    default:                     free_typed_resources(type_ptr, free_binary_res);       break;
    }
}

void cleanup_resources(void)
{
  int i;

  for (i = 0; i < MAX_TYPES; i++)
    free_res_type(resource_table + i);

  free_mem(resource_table);
}
