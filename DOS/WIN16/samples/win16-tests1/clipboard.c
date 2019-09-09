/*
 * Win16 Unit test suite for clipboard functions.
 *
 * Copyright 2007 Google, Jennifer Lai
 * Modified from /wine-git/dlls/user32/tests/clipboard.c by
 * Copyright 2002 Dmitry Timoshkov
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

#include "test.h"
#include <win16.h>

static void test_ClipboardOwner(void)
{
    HWND hWnd1, hWnd2;
    BOOL ret;

    SetLastError(0xdeadbeef);
    ok(!GetClipboardOwner() && GetLastError() == 0xdeadbeef,
       "could not perform clipboard test: clipboard already owned\n");

    hWnd1 = CreateWindow("STATIC", NULL, WS_POPUP,
                                 0, 0, 10, 10, 0, 0, 0, NULL);
    ok(hWnd1 != 0, "CreateWindow error %d\n", GetLastError());
    /* trace("hWnd1 = %p\n", hWnd1); */

    hWnd2 = CreateWindow("STATIC", NULL, WS_POPUP,
                                 0, 0, 10, 10, 0, 0, 0, NULL);
    ok(hWnd2 != 0, "CreateWindow error %d\n", GetLastError());
    /* trace("hWnd2 = %p\n", hWnd2); */

    SetLastError(0xdeadbeef);
    ok(!CloseClipboard(), "CloseClipboard should fail if clipboard wasn't open\n");

    ok(OpenClipboard(0), "OpenClipboard failed\n");
    ok(!GetClipboardOwner(), "clipboard should still be not owned\n");
    ok(!OpenClipboard(hWnd1), "OpenClipboard should fail since clipboard already opened\n");
    ret = CloseClipboard();
    ok( ret, "CloseClipboard error %d\n", GetLastError());

    ok(OpenClipboard(hWnd1), "OpenClipboard failed\n");

    SetLastError(0xdeadbeef);
    ok(!OpenClipboard(hWnd2) && GetLastError() == 0xdeadbeef,
       "OpenClipboard should fail without setting last error value\n");

    SetLastError(0xdeadbeef);
    ok(!GetClipboardOwner() && GetLastError() == 0xdeadbeef, "clipboard should still be not owned\n");
    ret = EmptyClipboard();
    ok( ret, "EmptyClipboard error %d\n", GetLastError());
    ok(GetClipboardOwner() == hWnd1, "clipboard should be owned by %p, not by %p\n", hWnd1, GetClipboardOwner());

    SetLastError(0xdeadbeef);
    ok(!OpenClipboard(hWnd2) && GetLastError() == 0xdeadbeef,
       "OpenClipboard should fail without setting last error value\n");

    ret = CloseClipboard();
    ok( ret, "CloseClipboard error %d\n", GetLastError());
    ok(GetClipboardOwner() == hWnd1, "clipboard should still be owned\n");

    ret = DestroyWindow(hWnd1);
    ok( ret, "DestroyWindow error %d\n", GetLastError());
    ret = DestroyWindow(hWnd2);
    ok( ret, "DestroyWindow error %d\n", GetLastError());
    SetLastError(0xdeadbeef);
    ok(!GetClipboardOwner() && GetLastError() == 0xdeadbeef, "clipboard should not be owned\n");
}

static void test_RegisterClipboardFormat(void)
{
    ATOM atom_id;
    UINT format_id, format_id2;
    char buf[256];
    int len;
    BOOL ret;

    format_id = RegisterClipboardFormat("my_cool_clipboard_format");
    ok(format_id > 0xc000 && format_id < 0xffff, "invalid clipboard format id %04x\n", format_id);

    format_id2 = RegisterClipboardFormat("MY_COOL_CLIPBOARD_FORMAT");
    ok(format_id2 == format_id, "invalid clipboard format id %04x\n", format_id2);

    len = GetClipboardFormatName(format_id, buf, 256);
    ok(len == lstrlen("my_cool_clipboard_format"), "wrong format name length %d\n", len);
    ok(!lstrcmp(buf, "my_cool_clipboard_format"), "wrong format name \"%s\"\n", buf);

    lstrcpy(buf, "foo");
    SetLastError(0xdeadbeef);
    len = GetAtomName((ATOM)format_id, buf, 256);
    ok(len == 0, "GetAtomName should fail\n");

todo_wine
{
    lstrcpy(buf, "foo");
    SetLastError(0xdeadbeef);
    len = GlobalGetAtomName((ATOM)format_id, buf, 256);
    ok(len == 0, "GlobalGetAtomNameA should fail\n");
}

    SetLastError(0xdeadbeef);
    atom_id = FindAtom("my_cool_clipboard_format");
    ok(atom_id == 0, "FindAtom should fail\n");

    ret = OpenClipboard(0);
    ok( ret, "OpenClipboard error %d\n", GetLastError());

    /* trace("# of formats available: %d\n", CountClipboardFormats()); */

    format_id = 0;
    while ((format_id = EnumClipboardFormats(format_id)))
    {
        ok(IsClipboardFormatAvailable(format_id), "format %04x was listed as available\n", format_id);
        len = GetClipboardFormatName(format_id, buf, 256);
        /* trace("%04x: %s\n", format_id, len ? buf : ""); */
    }

    ret = EmptyClipboard();
    ok( ret, "EmptyClipboard error %d\n", GetLastError());
    ret =CloseClipboard();
    ok( ret, "CloseClipboard error %d\n", GetLastError());

    if (CountClipboardFormats())
    {
        SetLastError(0xdeadbeef);
        ok(!EnumClipboardFormats(0), "EnumClipboardFormats should fail if clipboard wasn't open\n");
    }

    SetLastError(0xdeadbeef);
    ok(!EmptyClipboard(), "EmptyClipboard should fail if clipboard wasn't open\n");
}

START_TEST(clipboard)
{
    SetLastError(0xdeadbeef);
    FindAtom(NULL);

    test_RegisterClipboardFormat();
    test_ClipboardOwner();
}
