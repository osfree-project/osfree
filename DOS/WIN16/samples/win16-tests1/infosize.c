/* Win16 test suite for GetVersionInfoSize function
 * Copyright 2007 Google, Jennifer Lai
 * Modified from /wine-git/dlls/version/tests/info.c by
 * Copyright (C) 2004 Stefan Leichter
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

/* Win98 is used to verify tests */
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <ver.h>

#include "test.h"

#define MY_LAST_ERROR ((DWORD)-1)

static void test_info_size(void)
{   DWORD hdl, retval;

/* In Win16, the GetFileVersionInfoSize function opened the target file and
 * went searching for the version information. Once it was located, the size
 * of the version was the return value and the file offset of the version
 * information was stored in lpdwHandle.
 */
    SetLastError(MY_LAST_ERROR);
    retval = GetFileVersionInfoSize( "", NULL);
    ok( !retval,
        "GetFileVersionInfoSize result wrong! 0L expected, got 0x%08x\n",
        retval);

    hdl = 0x55555555;
    SetLastError(MY_LAST_ERROR);
    retval = GetFileVersionInfoSize( "", &hdl);
    ok( !retval,
        "GetFileVersionInfoSize result wrong! 0L expected, got 0x%08x\n",
        retval);


    SetLastError(MY_LAST_ERROR);
    retval = GetFileVersionInfoSize("shell.dll", &hdl);
    ok( retval,
        "GetFileVersionInfoSize result wrong! <> 0L expected, got 0x%08x\n",
        retval);
    ok(MY_LAST_ERROR == GetLastError(),
        "Last error wrong! NO_ERROR/0x%08x (NT4)  expected, got %u\n",
        MY_LAST_ERROR, GetLastError());

    SetLastError(MY_LAST_ERROR);
    retval = GetFileVersionInfoSize("notexist.dll", NULL);
    ok( !retval,
        "GetFileVersionInfoSize result wrong! 0L expected, got 0x%08x\n",
        retval);

    hdl = 0x55555555;
    SetLastError(MY_LAST_ERROR);
    retval = GetFileVersionInfoSize( "shell.dll", &hdl);
    ok( retval,
        "GetFileVersionInfoSize result wrong! <> 0L expected, got 0x%08x\n",
        retval);
    ok(MY_LAST_ERROR == GetLastError(),
        "Last error wrong! NO_ERROR/0x%08x (NT4)  expected, got %u\n",
        MY_LAST_ERROR, GetLastError());
    /* in win16, hdl is the file offset for the version information */
    todo_wine ok( (hdl > 0) && (hdl != 0x55555555),
        "Handle wrong! got 0x%08x\n", hdl);

    if ( retval == 0 || hdl != 0)
        return;
}

START_TEST(infosize)
{
    test_info_size();
}   
