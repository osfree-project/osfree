/*
 * Win16 Unit test suite for class functions.
 *
 * Copyright 2007 Jennfier Lai, Google
 * Modified from /wine-git/user32/tests/class.c written by
 * Mike McCormack, Copyright 2002
 * Alexandre Julliard, Copyright 2003
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

/* haven't figured out how to make SetClassWord() work properly for win16 and 
 * still having troubles with kernel module in XP, so test_instances() will be
 * added later...*/

#define STRICT
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include "test.h"


#define NUMCLASSWORDS 4

static LRESULT WINAPI ClassTest_WndProc(HWND hWnd, UINT msg, WPARAM wParam,
                                        LPARAM lParam)
{
    return DefWindowProc(hWnd, msg, wParam, lParam);
}


static void test_Class(HINSTANCE hInstance, BOOL global)
{
    WNDCLASS cls, wc;
    static const char className[] =
        { 'T', 'e', 's', 't', 'C', 'l', 'a', 's', 's', 0 };
    static const char winName[] =
        { 'W', 'i', 'n', 'C', 'l', 'a', 's', 's', 'T', 'e', 's', 't', 0 };
    ATOM test_atom;
    HWND hTestWnd;
    LONG i;
    char str[20];
    ATOM classatom;

    cls.style = CS_HREDRAW | CS_VREDRAW | (global ? CS_GLOBALCLASS : 0);
    cls.lpfnWndProc = ClassTest_WndProc;
    cls.cbClsExtra = NUMCLASSWORDS * sizeof(DWORD);
    cls.cbWndExtra = 12;
    cls.hInstance = hInstance;
    cls.hIcon = LoadIcon(0, (LPCSTR) IDI_APPLICATION);
    cls.hCursor = LoadCursor(0, (LPCSTR) IDC_ARROW);
    cls.hbrBackground = GetStockObject(WHITE_BRUSH);
    cls.lpszMenuName = 0;
    cls.lpszClassName = className;

    classatom = RegisterClass(&cls);
    if (!classatom)
        return;
    ok(classatom, "failed to register class\n");

    ok(!RegisterClass(&cls),
       "RegisterClass of the same class should fail for the second time\n");

    /* Setup windows */
    hTestWnd = CreateWindow(className, winName,
                            WS_OVERLAPPEDWINDOW + WS_HSCROLL + WS_VSCROLL,
                            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 0,
                            0, hInstance, 0);

    ok(hTestWnd != 0, "Failed to create window\n");

    /* test initial values of valid classwords */
    for (i = 0; i < NUMCLASSWORDS; i++) {
        SetLastError(0);
        ok(!GetClassLong(hTestWnd, i * sizeof(DWORD)),
           "GetClassLong initial value nonzero!\n");
        ok(!GetLastError(), "GetClassLong failed!\n");
    }

    /* set values of valid class words */
    for (i = 0; i < NUMCLASSWORDS; i++) {
        SetLastError(0);
        ok(!SetClassLong(hTestWnd, i * sizeof(DWORD), i + 1),
           "GetClassLong(%d) initial value nonzero!\n", i);
        ok(!GetLastError(), "SetClassLongW(%d) failed!\n", i);
    }

    /* test values of valid classwords that we set */
    for (i = 0; i < NUMCLASSWORDS; i++) {
        SetLastError(0);
        ok((i + 1) == GetClassLong(hTestWnd, i * sizeof(DWORD)),
           "GetClassLong value doesn't match what was set!\n");
        ok(!GetLastError(), "GetClassLongW failed!\n");
    }

    /* check GetClassName */
    i = GetClassName(hTestWnd, str, sizeof(str));
    ok(i == lstrlen(className), "GetClassName returned incorrect length\n");
    ok(!lstrcmp(className, str),
       "GetClassName returned incorrect name for this window's class\n");

    /* check GetClassInfo with our hInstance */
    if ((test_atom = GetClassInfo(hInstance, str, &wc))) {
        ok(test_atom == classatom, "class atom did not match\n");
        ok(wc.cbClsExtra == cls.cbClsExtra, "cbClsExtra did not match\n");
        ok(wc.cbWndExtra == cls.cbWndExtra, "cbWndExtra did not match\n");
        ok(wc.hbrBackground == cls.hbrBackground,
           "hbrBackground did not match\n");
        ok(wc.hCursor == cls.hCursor, "hCursor did not match\n");
        ok(wc.hInstance == cls.hInstance, "hInstance did not match\n");
    } else
        ok(FALSE, "GetClassInfo (hinstance) failed!\n");

    /* check GetClassInfo with zero hInstance */
    if (global) {
        if ((test_atom = GetClassInfo(0, str, &wc))) {
            ok(test_atom == classatom,
               "class atom did not match %x != %x\n", test_atom, classatom);
            ok(wc.cbClsExtra == cls.cbClsExtra,
               "cbClsExtra did not match %x!=%x\n", wc.cbClsExtra,
               cls.cbClsExtra);
            ok(wc.cbWndExtra == cls.cbWndExtra,
               "cbWndExtra did not match %x!=%x\n", wc.cbWndExtra,
               cls.cbWndExtra);
            ok(wc.hbrBackground == cls.hbrBackground,
               "hbrBackground did not match %p!=%p\n", wc.hbrBackground,
               cls.hbrBackground);
            ok(wc.hCursor == cls.hCursor, "hCursor did not match %p!=%p\n",
               wc.hCursor, cls.hCursor);
            ok(!wc.hInstance, "hInstance not zero for global class %p\n",
               wc.hInstance);
        } else
            ok(FALSE, "GetClassInfo (0) failed for global class!\n");
    } else {
        ok(!GetClassInfo(0, str, &wc),
           "GetClassInfo(0) succeeded for local class!\n");
    }

    ok(!UnregisterClass(className, hInstance),
       "Unregister class succeeded with window existing\n");

    ok(DestroyWindow(hTestWnd), "DestroyWindow() failed!\n");

    ok(UnregisterClass(className, hInstance), "UnregisterClass() failed\n");

    return;
}

static void check_style(const char *name, int must_exist, UINT style,
                        UINT ignore)
{
    WNDCLASS wc;

    if (GetClassInfo(0, name, &wc)) {
        ok(!(~wc.style & style & ~ignore),
           "System class %s is missing bits %x (%08x/%08x)\n", name,
           ~wc.style & style, wc.style, style);
        ok(!(wc.style & ~style),
           "System class %s has extra bits %x (%08x/%08x)\n", name,
           wc.style & ~style, wc.style, style);
    } else
        ok(!must_exist, "System class %s does not exist\n", name);
}

/* test styles of system classes */
static void test_styles(void)
{
    /* check style bits */
    check_style("Button", 1, CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
                0);
    check_style("ComboBox", 1,
                CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, 0);
    check_style("Edit", 1, CS_PARENTDC | CS_DBLCLKS, 0);
    check_style("ListBox", 1, CS_PARENTDC | CS_DBLCLKS, CS_PARENTDC /*FIXME*/);
    check_style("MDIClient", 1, 0, 0);
    check_style("ScrollBar", 1,
                CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, 0);
    check_style("Static", 1, CS_PARENTDC | CS_DBLCLKS, 0);
    check_style("ComboLBox", 1, CS_SAVEBITS | CS_DBLCLKS, 0);
    check_style("DDEMLEvent", 0, 0, 0);
    check_style("Message", 0, 0, 0);
    check_style("#32769", 1, CS_DBLCLKS, 0);    /* desktop */
    check_style("#32770", 1, CS_SAVEBITS | CS_DBLCLKS, 0);      /* dialog */
    todo_wine {
        check_style("#32771", 1, CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW, 0);
    }                           /* task switch */
    check_style("#32772", 1, 0, 0);     /* icon title */
}

START_TEST(class)
{
    HANDLE hInstance = GetModuleHandle("kernel");

    test_Class(hInstance, FALSE);
    test_Class(hInstance, TRUE);
    test_styles();
	//test_instances();
}
