/*    
	Strings.c	2.21
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

#include "windows.h"
#include "Log.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static UINT uCodePage = 0;

/* 1252 is the default US Windows ANSI code page */
static UINT SetCodePage(void)
{
    if ( !uCodePage )
	uCodePage = GetPrivateProfileInt("boot.description", "CodePage",
					1252, "system.ini");
    return uCodePage != 1252;
}

LPSTR WINAPI
AnsiNext(LPCSTR lpchCurrentChar)
{
    if (!lpchCurrentChar)
	return (LPSTR)0;

    if (*lpchCurrentChar) {
	if ( IsDBCSLeadByte(*lpchCurrentChar) )
	    return (LPSTR)(lpchCurrentChar+2);
	else
	    return (LPSTR)(lpchCurrentChar+1);
    }
    else
	return (LPSTR)lpchCurrentChar;
}

LPSTR WINAPI
AnsiPrev(LPCSTR lpchStart, LPCSTR lpchCurrentChar)
{
    LPSTR lpPrev = (LPSTR)lpchStart;
    LPSTR lpNext;

    if (lpchStart == lpchCurrentChar)
	return (LPSTR)lpchStart;

    if ( SetCodePage() ) {
	while ((lpNext = AnsiNext((LPCSTR)lpPrev)) != (LPSTR)lpchCurrentChar)
	    lpPrev = lpNext;
	return lpPrev;
    }

    return (LPSTR)(lpchCurrentChar-1);
}
LPSTR   WINAPI 
AnsiUpper(LPSTR lpstr)
{
	LPSTR p;
	if(HIWORD(lpstr) == 0)
		return (LPSTR) toupper(LOWORD((DWORD)lpstr));	
	p = lpstr;
	while((*p = toupper(*p))) p++;
	return lpstr;
}


LPSTR   WINAPI 
AnsiLower(LPSTR lpstr)
{
	LPSTR p;
	if(HIWORD(lpstr) == 0)
		return (LPSTR) tolower(LOWORD((DWORD)lpstr));	
	p = lpstr;
	while((*p = tolower(*p))) p++;
	return lpstr;
}

UINT    WINAPI 
AnsiUpperBuff(LPSTR lpstr, UINT n)
{
	UINT count = n;
	
	if(count == 0)
		count = 65536;	
	while(count) {
		*lpstr = toupper(*lpstr);	
		lpstr++;
		count--;
	}
	return n;
}

UINT    WINAPI 
AnsiLowerBuff(LPSTR lpstr, UINT n) 
{
	UINT count = n;
	
	if(count == 0)
		count = 65536;	
	while(count) {
		*lpstr = tolower(*lpstr);	
		lpstr++;
		count--;
	}
	return n;
}

BOOL    WINAPI 
IsCharAlpha(char ch)
{
	APISTR((LF_API,"IsCharAlpha(char=%c)\n",ch));
	return (BOOL) isalpha((int) ch);
}

BOOL    WINAPI 
IsCharAlphaNumeric(char ch)
{
	APISTR((LF_API,"IsCharAlphaNumeric(char=%c)\n",ch));
	return (BOOL) isalnum((int) ch);
}

BOOL    WINAPI 
IsCharUpper(char ch)
{
	APISTR((LF_API,"IsCharUpper(char %c)\n",ch));
	return (BOOL) isupper((int) ch);	
}

BOOL    WINAPI 
IsCharLower(char ch)
{
	APISTR((LF_API,"IsCharLower(char %c)\n",ch));
	return (BOOL) islower((int) ch);	
}

BOOL WINAPI
IsDBCSLeadByte(BYTE bTestChar)
{

    APISTR((LF_API,"IsDBCSLeadByte(char %c)\n",bTestChar));

	SetCodePage();

	switch (uCodePage) {

	    case 936:		/* Chinese (Simplified / Mainland) */
		if (bTestChar >= 0xA1 && bTestChar <= 0xFE)
		    return TRUE;
		break;

	    case 950:		/* Chinese (Traditional / Taiwan) */
		if (bTestChar >= 0x81 && bTestChar <= 0xFE)
		    return TRUE;
		break;

	    case 932:		/* Japanese (Shift-JIS) */
		if ((bTestChar >= 0x81 && bTestChar <= 0x9F) ||
		    (bTestChar >= 0xE0 && bTestChar <= 0xFC) )
		    return TRUE;
		break;

	    case 949:		/* Korean (Wansung) */
		if (bTestChar >= 0x81 && bTestChar <= 0xFE)
		    return TRUE;
		break;

	    case 1361:		/* Korean (Johab) */
		if ((bTestChar >= 0x84 && bTestChar <= 0xD3) ||
		    (bTestChar >= 0xD8 && bTestChar <= 0xDE) ||
		    (bTestChar >= 0xE0 && bTestChar <= 0xF9) )
		    return TRUE;
		break;

	}

	return FALSE;

}

void    WINAPI 
hmemcpy(void _huge* d, const void _huge* s, long n)
{
	memcpy(d,s,n);
}

LPSTR
itoa(int value, LPSTR string, int radix)
{
    char temp[256];
    int i=0, j=0;

    switch (radix) {
	case 2:
    sprintf(temp, "%x", value);
    for(i = 0; i < (int)strlen(temp); i++)
    {
      switch(temp[i])
      {
      case '0':
	string[j]   = '0';
	string[j+1] = '0';
	string[j+2] = '0';
	string[j+3] = '0';
	j += 4;
	break;
      case '1':
	string[j]   = '0';
	string[j+1] = '0';
	string[j+2] = '0';
	string[j+3] = '1';
	j += 4;
	break;
      case '2':
	string[j]   = '0';
	string[j+1] = '0';
	string[j+2] = '1';
	string[j+3] = '0';
	j += 4;
	break;
      case '3':
	string[j]   = '0';
	string[j+1] = '0';
	string[j+2] = '1';
	string[j+3] = '1';
	j += 4;
	break;
      case '4':
	string[j]   = '0';
	string[j+1] = '0';
	string[j+2] = '1';
	string[j+3] = '0';
	j += 4;
	break;
      case '5':
	string[j]   = '0';
	string[j+1] = '1';
	string[j+2] = '0';
	string[j+3] = '1';
	j += 4;
	break;
      case '6':
	string[j]   = '0';
	string[j+1] = '1';
	string[j+2] = '1';
	string[j+3] = '0';
	j += 4;
	break;
      case '7':
	string[j] = '0';
	string[j+1] = '1';
	string[j+2] = '1';
	string[j+3] = '1';
	j += 4;
	break;
      case '8':
	string[j]   = '1';
	string[j+1] = '0';
	string[j+2] = '0';
	string[j+3] = '0';
	j += 4;
	break;
      case '9':
       	string[j]   = '1';
	string[j+1] = '0';
	string[j+2] = '0';
	string[j+3] = '1';
	j += 4;
	break;
      case 'A': case 'a':
       	string[j]   = '1';
	string[j+1] = '0';
	string[j+2] = '1';
	string[j+3] = '0';
	j += 4;
	break;
      case 'B': case 'b':
       	string[j]   = '1';
	string[j+1] = '0';
	string[j+2] = '1';
	string[j+3] = '1';
	j += 4;
	break;
      case 'C': case 'c':
       	string[j]   = '1';
	string[j+1] = '1';
	string[j+2] = '0';
	string[j+3] = '0';
	j += 4;
	break;
      case 'D': case 'd':
       	string[j]   = '1';
	string[j+1] = '1';
	string[j+2] = '0';
	string[j+3] = '1';
	j += 4;
	break;
      case 'E': case 'e':
       	string[j]   = '1';
	string[j+1] = '1';
	string[j+2] = '1';
	string[j+3] = '0';
	j += 4;
	break;
      case 'F': case 'f':
       	string[j]   = '1';
	string[j+1] = '1';
	string[j+2] = '1';
	string[j+3] = '1';
	j += 4;
	break;
      }
    }
    string[j] = '\0';
    break;
	case 8:
	    sprintf(string,"%o",value);
	    break;
	case 10:
	    sprintf(string,"%d",value);
	    break;
	case 16:
	    sprintf(string,"%x",value);
	    break;
	default: 
	    sprintf(string,"%d[%d]",value,radix);
	    break;
	}

    return string;
}

LPSTR
strpbrkr(LPCSTR szStr, LPCSTR szTempl)
{
	int nLen, i, j, nEq = 0;

	nLen = strlen(szTempl);
	if (strlen(szStr) == 0)
	    	return (LPSTR)NULL;
	
	for ( i = strlen(szStr) - 1; i >= 0; i-- )
	    {
		for ( j = 0, nEq = 0; j < nLen; j++ )
			if ( *(szStr+i) == *(szTempl+j) )
			   {
				nEq++;
				break;
			   }
		if ( nEq )
			break;
		else
			nEq = 0;
	    }
	if ( nEq )
		return (LPSTR)(szStr+i);
	else
		return (LPSTR)NULL;
}
 
/*
	This routine is used to swap all occurences of a specific
	character in the first string with a replacement character.

	For example:	"\BIN\TEST.EXE" -> "/BIN/TEST.EXE"

		strswpchr("\BIN\TEST.EXE", '\\', '/');
*/
int
strswpchr(LPSTR szStr, char chFind, char chSwap)
{
	LPSTR szPtr;
	int Count = 0;

	szPtr = szStr;

	while ( *szPtr ) {
		if ( *szPtr == chFind ) {
			*szPtr = chSwap;
			Count++;
		}
		szPtr++;
	}

	return Count;
}

LPSTR WINAPI
lstrcpy(LPSTR lpszString1, LPCSTR lpszString2)
{
    if (!lpszString1 || !lpszString2)
	return (LPSTR)0;

    return strcpy(lpszString1,lpszString2);
}

LPSTR WINAPI
lstrcpyn(LPSTR lpszString1, LPCSTR lpszString2, int cChars)
{
    if (!lpszString1 || !lpszString2)
	return (LPSTR)0;

    strncpy(lpszString1,lpszString2,cChars);

    lpszString1[cChars-1] = '\0';	/* force null character */

    return lpszString1;
}

int WINAPI
lstrcmp(LPCSTR lpszString1,LPCSTR lpszString2)
{
    if (!lpszString1 || !lpszString2)
	return (lpszString2-lpszString1);

    LOGSTR((LF_API,"lstrcmp: %s %s\n",lpszString1,lpszString2));

    return strcmp(lpszString1,lpszString2);
}

int WINAPI
lstrcmpi(LPCSTR lpszString1,LPCSTR lpszString2)
{
    if (!lpszString1 || !lpszString2)
	return (lpszString2-lpszString1);

    LOGSTR((LF_API,"lstrcmpi: %s %s\n",lpszString1,lpszString2));

    return strcasecmp(lpszString1,lpszString2);
}

int WINAPI
lstrlen(LPCSTR lpszString)
{
    if (!lpszString)
	return 0;

    return strlen(lpszString);
}

LPSTR WINAPI
lstrcat(LPSTR lpszString1, LPCSTR lpszString2)
{
    return strcat(lpszString1,lpszString2);
}

/* (WIN32) Character Conversion ******************************************** */

LPTSTR	WINAPI
CharLower(LPTSTR lpstr)
{
	LPTSTR str;

	APISTR((LF_API, "CharLower: (API) lpstr %p\n", (void *) lpstr));

	if (HIWORD(lpstr) == 0)
		return ((LPTSTR) tolower(LOWORD((DWORD)lpstr)));	
	for (str = lpstr; (*str = tolower(*str)); str++)
		;
	return (lpstr);

}

LPTSTR	WINAPI
CharUpper(LPTSTR lpstr)
{
	LPTSTR str;

	APISTR((LF_API, "CharUpper: (API) lpstr %p\n", (void *) lpstr));

	if (HIWORD(lpstr) == 0)
		return ((LPTSTR) toupper(LOWORD((DWORD)lpstr)));	
	for (str = lpstr; (*str = toupper(*str)); str++)
		;
	return (lpstr);

}

DWORD	WINAPI
CharLowerBuff(LPTSTR lpstr, DWORD len)
{
	DWORD i;

	APISTR((LF_API, "CharLowerBuff: (API) lpstr %p len %ld\n",
		(void *) lpstr, len));

	if (!lpstr)
		return (0);
	for (i = 0; i < len; i++, lpstr++)
		*lpstr = tolower(*lpstr);
	return (i);

}

DWORD	WINAPI
CharUpperBuff(LPTSTR lpstr, DWORD len)
{
	DWORD i;

	APISTR((LF_API, "CharUpperBuff: (API) lpstr %p len %ld\n",
		(void *) lpstr, len));

	if (!lpstr)
		return (0);
	for (i = 0; i < len; i++, lpstr++)
		*lpstr = toupper(*lpstr);
	return (i);

}

LPTSTR	WINAPI
CharNext(LPCTSTR lpstr)
{
	APISTR((LF_API, "CharNext: (API) lpstr %p\n", (void *) lpstr));
	if (!lpstr)
		return ((LPTSTR) 0);
	if (*lpstr)
		return ((LPTSTR) (lpstr + 1));
	return ((LPTSTR) lpstr);
}

LPTSTR	WINAPI
CharPrev(LPCTSTR lpstr0, LPCTSTR lpstr)
{
	APISTR((LF_API, "CharPrev: (API) lpstr0 %p lpstr %p\n",
		(void *) lpstr0, (void *) lpstr));
	if (!lpstr0 || !lpstr)
		return ((LPTSTR) 0);
	if (lpstr0 == lpstr)
		return ((LPTSTR) lpstr);
	return ((LPTSTR) (lpstr - 1));
}



