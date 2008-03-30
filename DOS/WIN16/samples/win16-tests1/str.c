/*
 * Win16 Unit test suite for char/string/text functions.
 *
 * Copyright 2007 Jennfier Lai, Google
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */
#define STRICT

#include <windows.h>
#include "test.h"


struct _textAlign {
	UINT flag;
	UINT result1;
	UINT result2;
};

struct _textDrawn {
	UINT mode;
	UINT height;
};

/* various char tests */
static void test_IsChar(void)
{
	char a='A', b='5', c='$', d='p';
	
	ok(IsCharAlpha(a),"char test 1 failed!\n");
	ok(!IsCharAlpha(b),"char test 2 failed!\n");
	ok(!IsCharAlpha(c),"char test 3 failed!\n");
	ok(IsCharAlphaNumeric(b),"char test 4 failed!\n");
	ok(IsCharAlphaNumeric(a),"char test 5 failed!\n");
	ok(!IsCharAlphaNumeric(c),"char test 6 failed!\n");
	ok(!IsCharLower(a),"char test 7 failed!\n");
	ok(IsCharLower(d),"char test 8 failed!\n");
	ok(IsCharUpper(a),"char test 9 failed!\n");
	ok(!IsCharUpper(d),"char test 10 failed!\n");
}

/* test various string manipulations*/
static void test_StringManipulation(void) 
{
	LPSTR text1="helloworld!!!~~~";
	LPSTR text2="HeLLOworLd!!!~~~";
	LPSTR text3="Good";
	LPSTR text4="Bye";
	char buf[512];
	int a[3] = {5, 8, 13};
	int size;

	ok(lstrcmp(text1, text1) == 0, "lstrcmp() failed!\n");
	ok(lstrcmp(text1, text2) != 0, "lstrcmp() failed!\n");
	ok(lstrcmpi(text1, text1) == 0, "lstrcmpi() failed!\n");
	ok(lstrcmpi(text1, text2) == 0, "lstrcmpi() failed!\n");
	ok(lstrcmp(lstrcat(text3, text4), "GoodBye") == 0, "lstrcat() failed!\n");
	ok(lstrcmp(lstrcpy(text3, text4), "Bye") == 0, "lstrcpy() failed!\n");
	ok(lstrlen(text1)==16, "lstrlen() failed!\n");
	size = wsprintf(buf, "The sum of %d and %d is %d\n", 8, 2, 8+2);
	ok(lstrlen(buf) == size, "wsprintf returns wrong size!\n");
	memset(buf, 0, sizeof(buf));
	size = wvsprintf(buf, "The sum of %d and %d is %d\n", a);
	ok(lstrlen(buf) == size, "wvsprintf returns wrong size!\n");
}

/*  test various text alignment settings */
static void test_TextAlign(void)
{
	HDC hDC = GetDC(0);
	int i;
	
	struct _textAlign textAlign[]={
		{TA_LEFT | TA_TOP, 0, 0},
		{TA_LEFT | TA_TOP, 0, 0},
		{TA_RIGHT | TA_BOTTOM, 0, 0},
		{TA_CENTER | TA_BASELINE, 0, 0},
		{TA_LEFT | TA_BOTTOM, 0, 0},
	};
	
	for(i=0; i<5; i++){
		textAlign[i].result1 = GetTextAlign(hDC);
		if(i>0) {
			ok(textAlign[i].result1 == textAlign[i-1].flag, "Should get what was previously set\n");
		}
		textAlign[i].result2 = SetTextAlign(hDC, textAlign[i].flag);		
		ok(textAlign[i].result1 == textAlign[i].result2, "SetTextAlign should return the previously set align flag!, i=%d\n", i); 
	}
}

/*  test text color settings */
static void test_TextColor(void)
{
	HDC hDC = GetDC(0);
	COLORREF colors[] = {0, RGB(255,0,0), RGB(0,255,0), RGB(0,0,255), RGB(255,0,255), RGB(0,255,255), RGB(255,255,0), RGB(0,0,0), RGB(255,255,255)};
	int i;

	for(i=0; i<8; i++) {
		ok(GetTextColor(hDC) == colors[i], "Textcolor isn't what's set!\n");
		ok(SetTextColor(hDC, colors[i+1]) == colors[i], "SetTextColor should return the previously set color!");	
	}
}

/* test text extra space */ 
static void test_CharacterExtra(void)
{
	HDC hDC;
	int extra_space_previous, extra_space;
		
	hDC = GetDC(0);
	extra_space = 1;
	ok(!GetTextCharacterExtra(hDC), "GetTextCharacterExtra(hDC) should be null!\n");
	ok(!SetTextCharacterExtra(hDC, extra_space), "SetTextCharacterExtra(hDC) should be null!\n");
	extra_space_previous = extra_space;
	extra_space = 2;
	ok(GetTextCharacterExtra(hDC) == extra_space_previous, "GetTextCharacterExtra(hDC) should be null!\n");
	ok(SetTextCharacterExtra(hDC, extra_space) == extra_space_previous, "SetTextCharacterExtra(hDC) should be null!\n");
	extra_space_previous = extra_space;
	extra_space = 3;
	ok(GetTextCharacterExtra(hDC) == extra_space_previous, "GetTextCharacterExtra(hDC) should be null!\n");
	ok(SetTextCharacterExtra(hDC, extra_space) == extra_space_previous, "SetTextCharacterExtra(hDC) should be null!\n");
	extra_space_previous = extra_space;
	extra_space = 100;
	ok(GetTextCharacterExtra(hDC) == extra_space_previous, "GetTextCharacterExtra(hDC) should be null!\n");
	ok(SetTextCharacterExtra(hDC, extra_space) == extra_space_previous, "SetTextCharacterExtra(hDC) should be null!\n");
	extra_space_previous = extra_space;
	extra_space = -15;
	ok(GetTextCharacterExtra(hDC) == extra_space_previous, "GetTextCharacterExtra(hDC) should be null!\n");
	ok(SetTextCharacterExtra(hDC, extra_space) == extra_space_previous, "SetTextCharacterExtra(hDC) should be null!\n");
	
}

static void test_DrawText(void) 
{
	RECT rec;
	LPRECT pRec = &rec;
	HDC hDC;
	LPCSTR buf = "DrawText";
	int i;
	
	struct _textDrawn textDrawn[]={
		{DT_BOTTOM, 0},
		{DT_CALCRECT, 0},
		{DT_CENTER, 0},
		{DT_EXPANDTABS, 0},
		{DT_EXTERNALLEADING, 0},
		{DT_LEFT, 0},
		{DT_NOCLIP, 0},
		{DT_NOPREFIX, 0},
		{DT_RIGHT, 0},
		{DT_SINGLELINE, 0},
		{DT_TABSTOP, 0},
		{DT_TOP, 0},
		{DT_VCENTER, 0},
		{DT_WORDBREAK, 0},
	};
	
	hDC = GetDC(0);
	rec.left = 10;
	rec.top = 15;
	rec.right = 20;
	rec.bottom = 5;

	for(i=0; i<14; i++)
	{
	textDrawn[i].height = DrawText(hDC, buf, 8, pRec, textDrawn[i].mode);
	ok(textDrawn[i].height != NULL, "DrawText with flag %d failed!\n", textDrawn[i].mode);
	}
}

static void test_TextOut(void)
{
	HDC hDC;
	LPCSTR buf = "TextOut";
	BOOL ret;
	LONG ret_tabbed;
	int numTabs = 1;
	int *TabsArray = &numTabs;
	RECT rec;
	LPRECT pRec = &rec;
	
	hDC = GetDC(0);
	rec.left = 10;
	rec.top = 15;
	rec.right = 20;
	rec.bottom = 5;

	hDC = GetDC(hDC);
	ret = TextOut(hDC, 10, 10, buf, 8);
	ok(ret == TRUE, "TextOut failed!\n");

	ret = ExtTextOut(hDC, 10, 10, ETO_CLIPPED, pRec, buf, 8, NULL);
	ok(ret == TRUE, "TextOut with flag ETO_CLIPPED failed!\n");
	ret = ExtTextOut(hDC, 10, 10, ETO_OPAQUE, pRec, buf, 8, NULL);
	ok(ret == TRUE, "TextOut with flag ETO_OPAQUE failed!\n");
	
	ret_tabbed = TabbedTextOut(hDC, 5, 15, buf, 8, numTabs, TabsArray, 0); 
	ok(ret_tabbed > 0, "TabbedTextOut failed!\n");
}

static void test_GetTextExtent(void)
{
	HDC hDC;
	DWORD ret;
	LPCSTR buf = "TextExtent";
	int numTabs = 1;
	int * TabsArray = &numTabs;
	
	hDC = GetDC(0);
	ret = GetTextExtent(hDC, buf, 8);
	ok(ret > 0, "GetTextExtent failed!\n");
	
	ret = GetTabbedTextExtent(hDC, buf, 8, numTabs, TabsArray);	
	ok(ret > 0, "GetTabbedTextExtent failed!\n");
}


START_TEST(str)
{
	test_IsChar();
	test_StringManipulation();
	test_TextAlign();
	test_TextColor();
	test_CharacterExtra();
	test_DrawText();	
	test_TextOut();
	test_GetTextExtent();
}
