/*
 * Win16 Unit test suite for comdlg functions: printer dialogs
 *
 * Copyright 2007 Jennfier Lai, Google
 * Modified from /wine-git/dlls/comflg32/tests/filedlg.c
 * Copyright 2007 Google (Lei Zhang)
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
#include <commdlg.h>
#include <cderr.h>
#include "test.h"


static UINT CALLBACK OFNHookProc(HWND hDlg, UINT msg, WPARAM wParam,
                                 LPARAM lParam)
{
	if( msg == WM_INITDIALOG){
			PostMessage(hDlg, WM_COMMAND, IDCANCEL, FALSE);
	}
    return 0;
}


/* bug 6829 - fixed by Lei Zhang */
static void test_DialogCancel(void)
{
    OPENFILENAME ofn;
    BOOL result;
    char szFileName[MAX_PATH] = "";

	memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags =
        OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ENABLEHOOK;
    ofn.lpstrDefExt = "txt";
    ofn.lpfnHook = (LPOFNHOOKPROC) OFNHookProc;

    result = GetOpenFileName(&ofn);
    ok(0 == result, "expected %d, got %d\n", 0, result);
    ok(0 == CommDlgExtendedError(), "expected %d, got %d\n", 0,
       CommDlgExtendedError());
	
    SetLastError(0xdeadbeef);
    result = GetOpenFileName((LPOPENFILENAME) & ofn);
    ok(0 == result, "expected %d, got %d\n", 0, result);
    ok(0 == CommDlgExtendedError(), "expected %d, got %d\n", 0,
       CommDlgExtendedError());

    result = GetSaveFileName(&ofn);
    ok(0 == result, "expected %d, got %d\n", 0, result);
    ok(0 == CommDlgExtendedError(), "expected %d, got %d\n", 0,
       CommDlgExtendedError());

    SetLastError(0xdeadbeef);
    result = GetSaveFileName((LPOPENFILENAME) & ofn);
    ok(0 == result, "expected %d, got %d\n", 0, result);
    ok(0 == CommDlgExtendedError(), "expected %d, got %d\n", 0,
       CommDlgExtendedError());
}

START_TEST(filedlg)
{
    test_DialogCancel();
}
