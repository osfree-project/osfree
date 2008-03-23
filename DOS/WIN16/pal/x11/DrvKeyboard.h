/*
	@(#)DrvKeyboard.h	1.7
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
 
#define	KT_TYPE		0
#define	KT_SUBTYPE	1
#define	KT_NUMFKEYS	2

#define KS_UNSHIFTED    0
#define KS_SHIFTED      1

#ifndef OEMVK_KEYS
#define OEMVK_KEYS

#define VK_1	0x31
#define VK_2	0x32
#define VK_3	0x33
#define VK_4	0x34
#define VK_5	0x35
#define VK_6	0x36
#define VK_7	0x37
#define VK_8	0x38
#define VK_9	0x39
#define VK_0	0x30

#define VK_A	0x41
#define VK_B	0x42
#define VK_C	0x43
#define VK_D	0x44
#define VK_E	0x45
#define VK_F	0x46
#define VK_G	0x47
#define VK_H	0x48
#define VK_I	0x49
#define VK_J	0x4a
#define VK_K	0x4b
#define VK_L	0x4c
#define VK_M	0x4d
#define VK_N	0x4e
#define VK_O	0x4f
#define VK_P	0x50
#define VK_Q	0x51
#define VK_R	0x52
#define VK_S	0x53
#define VK_T	0x54
#define VK_U	0x55
#define VK_V	0x56
#define VK_W	0x57
#define VK_X	0x58
#define VK_Y	0x59
#define VK_Z	0x5a

#define VK_OEM_SCROLL	0x91

#define VK_OEM_1	0xba
#define VK_OEM_PLUS	0xbb
#define VK_OEM_COMMA	0xbc
#define VK_OEM_MINUS	0xbd
#define VK_OEM_PERIOD	0xbe
#define VK_OEM_2	0xbf

#define VK_OEM_3	0xc0

#define VK_OEM_4	0xdb
#define VK_OEM_5	0xdc
#define VK_OEM_6	0xdd
#define VK_OEM_7	0xde
#define VK_OEM_8	0xdf

#define VK_ALT		0x2a  /* OEM specific key, to map to external ALT */
#endif

/* virtual key array: maps vk_keys to ascii keys */
typedef struct tagKEYSTATE
{
    UINT	virtual;	/* vk keys to return based on keycode 	*/
    UINT	keypad;		/* vk keys if NUMLOCK 			*/

    UINT	vkUnshifted;	/* wm_char to return for given vk_key   */
    UINT	vkShifted;	
    UINT	vkUnshiftedCtrl;/* wm_char when control key is also down*/
    UINT	vkShiftedCtrl;	/* wm_char when control key is also down*/

    UINT 	vk_state;	/* state of vk_key */
} KEYSTATE;

#define KS_UP		0x0	/* Special key is up */
#define KS_DN		0x1	/* Special key is down */
#define KS_LK		0x2	/* Special key is locked */

typedef struct tagKEYTABLE
{
    UINT	FromCode;
    UINT	ToCode;
} KEYTABLE;

void DrvSynchronizeAllModifiers(unsigned int state);
