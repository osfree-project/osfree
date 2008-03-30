/* Win16 Unit test suite for window functions.
 *
 * Copyright 2007 Jennfier Lai, Google
 * Modified from /wine-git/user32/tests/win.c written by
 * Bill Medland, Copyright 2002.
 * Alexandre Julliard, Copyright 2002.
 * Dmitry Timoshkov, Copyright 2003.
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

#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <windows.h>
#include <commdlg.h>
#include "test.h"

static HWND create_tool_window(LONG style, HWND parent)
{
    HWND ret =
        CreateWindow("BUTTON", "ToolWindow1", style, 0, 0, 100, 100, parent, 0,
                     0, NULL);
    ok(ret != 0, "Creation failed\n");
    return ret;
}

static void test_Window(void)
{
    DWORD res, res1;
    HWND hwnd, hwnd2, ret, test, desktop, child;
    HINSTANCE hInst = NULL;
    hwnd =
        CreateWindow("BUTTON", "Test1", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
                     hInst, NULL);
    res = GetWindowLong(hwnd, GWL_STYLE);

    /* should get previous value */
    res1 = SetWindowLong(hwnd, GWL_STYLE, res);
    ok(res1 == res, "SetWindoLong failed! %d %d\n", res1, res);

    /* Visibility */
    SetWindowLong(hwnd, GWL_STYLE, res | WS_VISIBLE);   /*making it visible */
    ok(IsWindowVisible(hwnd) == TRUE, "window should be visible.\n");;
    SetWindowLong(hwnd, GWL_STYLE, res | ~WS_VISIBLE);  /*making it invisible */
    ok(IsWindowVisible(hwnd) == FALSE, "window should not be visible.\n");
    hwnd2 =
        CreateWindow("BUTTON", "Test2", WS_POPUPWINDOW, CW_USEDEFAULT,
                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
                     hInst, NULL);
    res = BringWindowToTop(hwnd2);
    ok(res == TRUE, "BringWindowToTop failed!\n");
    CloseWindow(hwnd2);         /* minimize it */
    ok(IsIconic(hwnd2) == TRUE, "IsIconic failed!\n");
    ok(IsIconic(NULL) == FALSE, "IsIconic(NULL) failed!\n");
    ok(IsZoomed(hwnd2) == FALSE, "IsZoomed failed!\n");
    ok(IsZoomed(NULL) == FALSE, "IsZoomed failed!\n");
    res = OpenIcon(hwnd2);      /*activate given minimized window */
    ok(res == TRUE, "OpenIcon failed!\n");
    res = BringWindowToTop(NULL);
    ok(res == FALSE, "BringWindowToTop(NULL) failed!\n");
    res = BringWindowToTop(hwnd2);
    EnableWindow(hwnd2, FALSE);
    ok(IsWindowEnabled(hwnd2) == FALSE, "IsWindowEnabled(FALSE) failed! \n");
    EnableWindow(hwnd2, TRUE);
    ok(IsWindowEnabled(hwnd2) == TRUE, "IsWindowEnabled(TRUE) failed! \n");
    ok(IsWindow(hwnd2) == TRUE, "IsWindow failed!\n");
    ok(GetActiveWindow() == SetActiveWindow(hwnd), "SetActiveWindow failed!\n");
    ok(GetActiveWindow() == hwnd, "GetActiveWindow failed!\n");

    /* Capture */
    ok(GetCapture() == SetCapture(hwnd), "SetCapture failed!\n");
    ok(GetCapture() == hwnd, "GetCapture failed!\n");
    ReleaseCapture();
    ok(GetCapture() == NULL, "ReleaseCapture() failed!\n");

    /* Focus */
    SetFocus(hwnd2);
    ok(GetFocus() == hwnd2, "SetFocus failed!\n");
    ok(SetFocus(NULL) == hwnd2, "SetFocus(NULL) failed!\n");

    /* Normal child Window */
    test = create_tool_window(WS_CHILD, hwnd2);
    desktop = GetDesktopWindow();
    child = create_tool_window(WS_CHILD, hwnd2);
    ret = SetParent(test, desktop);
    ok(ret == hwnd2, "SetParent return value %p expected %p\n", ret, hwnd2);
    ret = SetParent(test, child);
    ok(ret == desktop, "SetParent return value %p expected %p\n", ret, desktop);
    ret = SetParent(test, 0);
    ok(ret == child, "SetParent return value %p expected %p\n", ret, child);
    ok(DestroyWindow(test) == TRUE, "DestroyWindow(test) failed!\n");

    /* not owned top-level window */
    test = create_tool_window(0, 0);
    ret = SetParent(test, child);
    ok(ret == desktop, "SetParent return value %p expected %p\n", ret, desktop);
    ok(DestroyWindow(test) == TRUE, "DestroyWindow(test) failed!\n");

    /* owned popup */
    test = create_tool_window(WS_POPUP, hwnd2);
    ret = SetParent(test, child);
    ok(ret == desktop, "SetParent return value %p expected %p\n", ret, desktop);
    DestroyWindow(test);
    ok(DestroyWindow(hwnd) == TRUE, "DestroyWindow(hwnd) failed!\n");
    ok(DestroyWindow(hwnd2) == TRUE, "DestroyWindow(hwnd2) failed!\n");
}

static void test_ShowWindow(void)
{
    HWND hwnd;
    DWORD style;
    RECT rcMain, rc;
    LPARAM ret;
    SetRect(&rcMain, 120, 120, 210, 210);
    hwnd =
        CreateWindow("EDIT", NULL,
                     WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
                     | WS_POPUP, rcMain.left, rcMain.top,
                     rcMain.right - rcMain.left, rcMain.bottom - rcMain.top,
                     0, 0, 0, NULL);
    assert(hwnd);
    style = GetWindowLong(hwnd, GWL_STYLE);
    ok(!(style & WS_DISABLED), "window should not be disabled\n");
    ok(!(style & WS_VISIBLE), "window should not be visible\n");
    ok(!(style & WS_MINIMIZE), "window should not be minimized\n");
    ok(!(style & WS_MAXIMIZE), "window should not be maximized\n");
    GetWindowRect(hwnd, &rc);
    ok(EqualRect(&rcMain, &rc), "rects should match\n");
    ret = ShowWindow(hwnd, SW_SHOW);
    ok(!ret, "not expected ret: %lu\n", ret);
    style = GetWindowLong(hwnd, GWL_STYLE);
    ok(!(style & WS_DISABLED), "window should not be disabled\n");
    ok(style & WS_VISIBLE, "window should be visible\n");       /* broke??? */
    ok(!(style & WS_MINIMIZE), "window should not be minimized\n");
    ok(!(style & WS_MAXIMIZE), "window should not be maximized\n");
    GetWindowRect(hwnd, &rc);
    ok(EqualRect(&rcMain, &rc), "rects should match\n");
    ret = ShowWindow(hwnd, SW_MINIMIZE);
    ok(ret, "not expected ret: %lu\n", ret);
    style = GetWindowLong(hwnd, GWL_STYLE);
    ok(!(style & WS_DISABLED), "window should not be disabled\n");
    ok(style & WS_VISIBLE, "window should be visible\n");       /* broke too */
    ok(style & WS_MINIMIZE, "window should be minimized\n");    /* broke too */
    ok(!(style & WS_MAXIMIZE), "window should not be maximized\n");
    GetWindowRect(hwnd, &rc);
    ok(!EqualRect(&rcMain, &rc), "rects shouldn't match\n");
    ShowWindow(hwnd, SW_RESTORE);
    ok(ret, "not expected ret: %lu\n", ret);
    style = GetWindowLong(hwnd, GWL_STYLE);
    ok(!(style & WS_DISABLED), "window should not be disabled\n");
    ok(style & WS_VISIBLE, "window should be visible\n");
    ok(!(style & WS_MINIMIZE), "window should not be minimized\n");
    ok(!(style & WS_MAXIMIZE), "window should not be maximized\n");
    GetWindowRect(hwnd, &rc);
    ok(EqualRect(&rcMain, &rc), "rects should match\n");
    ret = EnableWindow(hwnd, FALSE);
    ok(!ret, "not expected ret: %lu\n", ret);
    style = GetWindowLong(hwnd, GWL_STYLE);
    ok(style & WS_DISABLED, "window should be disabled\n");
    ret = DefWindowProc(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
    ok(!ret, "not expected ret: %lu\n", ret);
    style = GetWindowLong(hwnd, GWL_STYLE);
    ok(style & WS_DISABLED, "window should be disabled\n");
    ok(style & WS_VISIBLE, "window should be visible\n");
    ok(!(style & WS_MINIMIZE), "window should not be minimized\n");
    ok(!(style & WS_MAXIMIZE), "window should not be maximized\n");
    GetWindowRect(hwnd, &rc);
    ok(EqualRect(&rcMain, &rc), "rects should match\n");
    ret = DefWindowProc(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    ok(!ret, "not expected ret: %lu\n", ret);
    style = GetWindowLong(hwnd, GWL_STYLE);
    ok(style & WS_DISABLED, "window should be disabled\n");
    ok(style & WS_VISIBLE, "window should be visible\n");
    ok(!(style & WS_MINIMIZE), "window should not be minimized\n");
    ok(!(style & WS_MAXIMIZE), "window should not be maximized\n");
    GetWindowRect(hwnd, &rc);
    ok(EqualRect(&rcMain, &rc), "rects should match\n");
    ret = ShowWindow(hwnd, SW_MINIMIZE);
    ok(ret, "not expected ret: %lu\n", ret);
    style = GetWindowLong(hwnd, GWL_STYLE);
    ok(style & WS_DISABLED, "window should be disabled\n");
    ok(style & WS_VISIBLE, "window should be visible\n");
    ok(style & WS_MINIMIZE, "window should be minimized\n");
    ok(!(style & WS_MAXIMIZE), "window should not be maximized\n");
    GetWindowRect(hwnd, &rc);
    ok(!EqualRect(&rcMain, &rc), "rects shouldn't match\n");
    ret = DefWindowProc(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    ok(!ret, "not expected ret: %lu\n", ret);
    style = GetWindowLong(hwnd, GWL_STYLE);
    ok(style & WS_DISABLED, "window should be disabled\n");
    ok(style & WS_VISIBLE, "window should be visible\n");
    ok(style & WS_MINIMIZE, "window should be minimized\n");
    ok(!(style & WS_MAXIMIZE), "window should not be maximized\n");
    GetWindowRect(hwnd, &rc);
    ok(!EqualRect(&rcMain, &rc), "rects shouldn't match\n");
    ret = ShowWindow(hwnd, SW_RESTORE);
    ok(ret, "not expected ret: %lu\n", ret);
    style = GetWindowLong(hwnd, GWL_STYLE);
    ok(style & WS_DISABLED, "window should be disabled\n");
    ok(style & WS_VISIBLE, "window should be visible\n");
    ok(!(style & WS_MINIMIZE), "window should not be minimized\n");
    ok(!(style & WS_MAXIMIZE), "window should not be maximized\n");
    GetWindowRect(hwnd, &rc);
    ok(EqualRect(&rcMain, &rc), "rects should match\n");
    ret = DefWindowProc(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
    ok(!ret, "not expected ret: %lu\n", ret);
    ok(IsWindow(hwnd), "window should exist\n");
    ret = EnableWindow(hwnd, TRUE);
    ok(ret, "not expected ret: %lu\n", ret);
    ret = DefWindowProc(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
    ok(!ret, "not expected ret: %lu\n", ret);
    ok(!IsWindow(hwnd), "window should not exist\n");
}

static void test_GetUpdateRect(void)
{
    RECT rc1, rc2;
    HWND hgrandparent, hparent, hchild;
    WNDCLASS cls;
    const char *classNameA = "GetUpdateRectClass";
    hgrandparent =
        CreateWindow("static", "grandparent", WS_OVERLAPPEDWINDOW, 0, 0, 100,
                     100, NULL, NULL, 0, NULL);
    hparent =
        CreateWindow("static", "parent", WS_CHILD | WS_VISIBLE, 0, 0, 100, 100,
                     hgrandparent, NULL, 0, NULL);
    hchild =
        CreateWindow("static", "child", WS_CHILD | WS_VISIBLE, 10, 10, 30, 30,
                     hparent, NULL, 0, NULL);
    ShowWindow(hgrandparent, SW_SHOW);
    UpdateWindow(hgrandparent);
    ShowWindow(hchild, SW_HIDE);
    SetRect(&rc2, 0, 0, 0, 0);
    GetUpdateRect(hgrandparent, &rc1, FALSE);
    ok(EqualRect(&rc1, &rc2),
       "rects do not match (%d,%d,%d,%d) / (%d,%d,%d,%d)\n", rc1.left,
       rc1.top, rc1.right, rc1.bottom, rc2.left, rc2.top, rc2.right,
       rc2.bottom);
    SetRect(&rc2, 10, 10, 40, 40);
    GetUpdateRect(hparent, &rc1, FALSE);
    ok(EqualRect(&rc1, &rc2),
       "rects do not match (%d,%d,%d,%d) / (%d,%d,%d,%d)\n", rc1.left,
       rc1.top, rc1.right, rc1.bottom, rc2.left, rc2.top, rc2.right,
       rc2.bottom);
    DestroyWindow(hgrandparent);
    cls.style = 0;
    cls.lpfnWndProc = DefWindowProc;
    cls.cbClsExtra = 0;
    cls.cbWndExtra = 0;
    cls.hInstance = GetModuleHandle(0);
    cls.hIcon = 0;
    cls.hCursor = LoadCursor(0, (LPSTR) IDC_ARROW);
    cls.hbrBackground = GetStockObject(WHITE_BRUSH);
    cls.lpszMenuName = NULL;
    cls.lpszClassName = classNameA;
    if (!RegisterClass(&cls)) {
        UnregisterClass(classNameA, GetModuleHandle(0));        /*unregister previously registered class */
        trace("Register failed %d\n", GetLastError());
        return;
    }
    hgrandparent =
        CreateWindow("static", "grandparent", WS_OVERLAPPEDWINDOW, 0, 0, 100,
                     100, NULL, NULL, 0, NULL);
    hparent =
        CreateWindow(classNameA, "parent", WS_CHILD | WS_VISIBLE, 0, 0, 100,
                     100, hgrandparent, NULL, 0, NULL);
    hchild =
        CreateWindow(classNameA, "child", WS_CHILD | WS_VISIBLE, 10, 10, 30, 30,
                     hparent, NULL, 0, NULL);
    ShowWindow(hgrandparent, SW_SHOW);
    UpdateWindow(hgrandparent);
    ShowWindow(hchild, SW_HIDE);
    SetRect(&rc2, 0, 0, 0, 0);
    GetUpdateRect(hgrandparent, &rc1, FALSE);
    ok(EqualRect(&rc1, &rc2),
       "rects do not match (%d,%d,%d,%d) / (%d,%d,%d,%d)\n", rc1.left,
       rc1.top, rc1.right, rc1.bottom, rc2.left, rc2.top, rc2.right,
       rc2.bottom);
    SetRect(&rc2, 10, 10, 40, 40);
    GetUpdateRect(hparent, &rc1, FALSE);
    ok(EqualRect(&rc1, &rc2),
       "rects do not match (%d,%d,%d,%d) / (%d,%d,%d,%d)\n", rc1.left,
       rc1.top, rc1.right, rc1.bottom, rc2.left, rc2.top, rc2.right,
       rc2.bottom);
    DestroyWindow(hgrandparent);
}

START_TEST(window)
{
    test_Window();
    test_ShowWindow();
    test_GetUpdateRect();
}
