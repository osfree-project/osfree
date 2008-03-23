

#include "windows.h"
#include "Log.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

void    WINAPI 
AnsiToOemBuff(LPCSTR lpstr, LPSTR lpdest, UINT n)
{
	if(lpstr != lpdest)
		strncpy(lpdest,lpstr,n);	
	return;
}

void    WINAPI 
OemToAnsiBuff(LPCSTR lpstr, LPSTR lpdest, UINT n)
{
	if(lpstr != lpdest)
		strncpy(lpdest,lpstr,n);	
	return;
}

void    WINAPI 
AnsiToOem(const char _huge* lpsrc, char _huge* lpdest)
{
	if(lpsrc != lpdest)
		strcpy(lpdest,lpsrc);	
	return;
}

void WINAPI
OemToAnsi(LPCSTR lpszOemStr, LPSTR lpszWindowsStr)
{
    if (lpszWindowsStr != lpszOemStr)
	lstrcpy(lpszWindowsStr,lpszOemStr);
}

DWORD
OemKeyScan(UINT uOemChar)
{
    APISTR((LF_API,"OemKeyScan: STUB char %x\n",uOemChar));

    return (DWORD)-1;
}

/*
 *	from keyboard driver...
 *	add the following:
 *		input	output
 *		0	-1
 *		1	'A' + CONTROL
 *		2	'B' + CONTROL
 *		3	3
 *		4-7	'D' + CONTROL, 'E', 'F', 'G'	
 *		8	8
 *		9	9
 *		a	0xd + CONTROL
 *		b,c	'K' + CONTROL, 'L'
 *		d	0xd
 *		e-1a	'N' + CONTROL, 'LETTER' + CONTROL
 *		1e,1f	-1
 *		20-3f	VK_KEY w/wo shifts 'A'...
 *		40-5a	letter + SHIFT
 *		5b-60	VKKEY w/wo shifts
 *		60-7a	'A'-'Z' w/o shifts
 *		7b	0xdb
 *		7c 	0xdd
 *		7d	0xc0
 *		7e	0x08	+ CONTROL
 *		7f	0x08	+ CONTROL
 */

UINT WINAPI
VkKeyScan(UINT uChar)
{
	short	rc;

    	APISTR((LF_API,"VkKeyScan(char:%x)\n",uChar));

	rc = uChar & 0xff;

	/* A-Z VK is ascii code + shifted bit in upper byte */
	/* +0x100	vk is ascii code + shift key */
	if(rc >= 'A' && rc <= 'Z')
		return rc + 0x100;

	/* a-z VK is ascii code - 32 */
	/* +0x000	vk is ascii code */
	if(rc >= 'a' && rc <= 'z')
		return rc - 32;

	/* we need to calculate the remaining codes */
	/* +0x200	vk is ascii code + control key */
	/* +0x600	vk is ascii code + control&alt key */
	/* +0x700	vk is ascii code + shift&control&alt key */
	/* 0x300,0x400,0x500 are not used... */
	return -1;
}

