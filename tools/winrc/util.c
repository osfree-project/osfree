/*    
	util.c	2.7
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
#include <ctype.h>

#include "windows.h"
#include "Resources.h"
#include "rc.h"
#include "restypes.h"

#define	ADD_TO_LIST(root, new, type)		\
	{				\
	type *current;			\
	current = root;			\
	new->next = (type *)NULL;	\
	while (current && current->next)\
		current = current->next;\
	if (!current) 			\
		root = new;		\
	else				\
		current->next = new;	\
	}

#ifndef TRUE
#define	TRUE	    1
#define	FALSE	0
#endif

#define	MAX_TYPES	32

extern NAMEENTRY_NODE *name_table;


/*
 *  Gets a word from the byte stream.
 */
WORD
get_word(BYTE *ptr)
{
        WORD    retval;
#ifdef  FAST_ACCESS_AND_NOSWAP
        retval = *(WORD *)ptr;
#else
        retval =  ((WORD)(ptr[1]) << 8) + ptr[0];
#endif
        return (retval);
}
 
/*
 * Gets a DWORD from the byte stream using get_word calls.
 */
DWORD
get_dword(BYTE *ptr)
{
        DWORD retval;
#ifdef  FAST_ACCESS_AND_NOSWAP
        retval = *(DWORD *)ptr;
#else
        retval = ((DWORD)get_word(&ptr[2]) << 16) + (DWORD)get_word(ptr);
#endif
        return (retval);
}



/* 
 * calculates (!?) the number of rgb quads reqd. to represent
 * the pixel based on the bit count.
*/
WORD
bit_to_rgb(WORD bitcount)
{
                switch (bitcount) {
                        case 1:
                                return (2);
                        case 4:
                                return (16);
                        case 8:
                                return (256);
                        case 24:
                                return (0);
                }
	return (0);
}

	
void
pascal_to_asciiz(char *str)
{
	BYTE count, i;
	
	count = (int)((BYTE)str[0]);

	for (i = 0; i < count; i++)
		str[i] = str[i + 1];
	str[i] = 0;
}

void
asciiz_to_pascal(char *str)
{
	BYTE count, i;

	count = strlen(str);
	
	for (i = count; i > 0; i--)
		str[i] = str[i - 1];  
	str[i] = count;
}

 
/*
 *  returns  a pointer to child (popup) menu. if any. Returns NULL
 * otherwise.
 */
MENU_NODE *
get_child(MENU_NODE **root)
{
        MENU_NODE *current;
        if (*root == (MENU_NODE *) NULL)
                return (*root);
        current = *root;
        while(current && (current->child == (MENU_NODE *)NULL))
                current = current->next;
        if (current) {
		*root = current;
                return (current->child);
        } else
                return (MENU_NODE *)NULL;
}
 




char	*
search_namelist_type(WORD res_type)
{
	NAMEENTRY_NODE *temp;

	if ((int)(res_type  & 0x7fff) < (int)0x10)
		return ((char *)((DWORD)res_type));
		
	temp = name_table;	
	
	while (temp != (NAMEENTRY_NODE *) NULL) {
		if ((temp->data.wTypeOrdinal & 0x7fff) == (res_type & 0x7fff))
			return (temp->data.szType);
		temp = temp->next;
	}
	return ((char *)((DWORD)res_type));
}

void
incr_res_count(char *typename, NAMEINFO_NODE *info)
{
	TYPEINFO *type_ptr;
	NAMEINFO_NODE *root;
	int	i;
	int	search_by_name = FALSE;
	int	found = FALSE;
	char	*rcs_type;
	
	if (HIWORD((DWORD)typename) == 0)
		rcs_type = (char *)search_namelist_type(LOWORD((DWORD)typename));
	else
		rcs_type = typename;
	if (HIWORD((DWORD)rcs_type) != 0)
		search_by_name = TRUE;

	info->data.hRsrc = 0;
	info->data.hObject = 0;
	type_ptr = resource_table;
	for (i = 0; i < MAX_TYPES; i++) {
		if (type_ptr->rcscount == 0)
			break;
		if (search_by_name) {
			if (HIWORD(type_ptr->rcstypename) != 0)
				found = !strcmp(rcs_type, type_ptr->rcstypename);
		} else {
			if (HIWORD(type_ptr->rcstypename) == 0)
				found = (rcs_type == type_ptr->rcstypename);
		}
		if (found == TRUE)
			break;
		type_ptr++;
	}
	root = (NAMEINFO_NODE *) type_ptr->rcsinfo;
	ADD_TO_LIST(root, info, NAMEINFO_NODE);
	type_ptr->rcsinfo = (NAMEINFO *)root; 
	type_ptr->rcscount++;
	type_ptr->rcstypename = rcs_type;
}


void
get_bmp_info_header(BITMAPINFOHEADER *bmp_info_header, BYTE *ptr)
{
	bmp_info_header->biSize = get_dword(&ptr[0]);
	if (bmp_info_header->biSize == 40) {
		bmp_info_header->biWidth = get_dword(&ptr[4]);
		bmp_info_header->biHeight = get_dword(&ptr[8]);
		bmp_info_header->biPlanes = get_word(&ptr[12]);
		bmp_info_header->biBitCount = get_word(&ptr[14]);
		bmp_info_header->biCompression = get_dword(&ptr[16]);
		bmp_info_header->biSizeImage = get_dword(&ptr[20]);
		bmp_info_header->biXPelsPerMeter = get_dword(&ptr[24]);
		bmp_info_header->biYPelsPerMeter = get_dword(&ptr[28]);
		bmp_info_header->biClrUsed = get_dword(&ptr[32]);
		bmp_info_header->biClrImportant = get_dword(&ptr[36]);
	} else if (bmp_info_header->biSize == 12) {
#if 0
		bmp_info_header->biSize = 40; 
#endif
		bmp_info_header->biWidth = get_word(&ptr[4]);
		bmp_info_header->biHeight = get_word(&ptr[6]);
		bmp_info_header->biPlanes = get_word(&ptr[8]);
		bmp_info_header->biBitCount = get_word(&ptr[10]);
		bmp_info_header->biCompression = 0;
		bmp_info_header->biSizeImage = 0;
		bmp_info_header->biXPelsPerMeter = 0;
		bmp_info_header->biYPelsPerMeter = 0;
		bmp_info_header->biClrUsed = 0;
		bmp_info_header->biClrImportant = 0;
	} else {
		FATAL(("unknown type of bitmap"));
	}
	if (bmp_info_header->biCompression) {
		return;
	}
	if (bmp_info_header->biBitCount == 1) {
		bmp_info_header->biSizeImage = 
		(bmp_info_header->biWidth + 31)/32;
		bmp_info_header->biSizeImage <<= 2;
		bmp_info_header->biSizeImage *= 
			bmp_info_header->biHeight;
		bmp_info_header->biClrUsed = 2;
		bmp_info_header->biClrImportant = 2;
	} else if (bmp_info_header->biBitCount == 4) {
		bmp_info_header->biSizeImage = 
		(bmp_info_header->biWidth + 7)/8;
		bmp_info_header->biSizeImage <<= 2;
		bmp_info_header->biSizeImage *= 
			bmp_info_header->biHeight;
		bmp_info_header->biClrUsed = 0;
		bmp_info_header->biClrImportant = 16;
	} else if (bmp_info_header->biBitCount == 8) {
		bmp_info_header->biSizeImage = 
		(bmp_info_header->biWidth + 3)/4;
		bmp_info_header->biSizeImage <<= 2;
		bmp_info_header->biSizeImage *= 
			bmp_info_header->biHeight;
		bmp_info_header->biClrUsed = 0;
		bmp_info_header->biClrImportant = 256;
	}
}

int istrcmp(const char *str1, const char *str2)
{
  int i = 0;
  while (str1 && str2 && !(i = toupper(*str1++) - toupper(*str2++))) ;
  if (str1)
    if (str2)
      return i;
    else
      return *str1;
  else
    if (str2)
      return *str2;
    else
      return 0;
}
