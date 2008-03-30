/*
 * Win16 Unit test suite for system service functions.
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

#include <string.h>
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <ver.h>
#include <toolhelp.h>

#include "test.h"


typedef struct _LOADPARMS {
    WORD segEnv;
    LPSTR lpszCmdLine;
    UINT *lpShow;
    UINT *lpReserved;
} LOADPARMS;

static void test_GetVersion(void)
{
    DWORD version;
    version = GetVersion();
    ok(version > 0, "GetVersion error!\n");
}

static void test_Library(void)
{
    HINSTANCE hInstance;

    /* check NULL parameters */
    hInstance = LoadLibrary(NULL);
    ok(hInstance == 0, "LoadLibrary(NULL) failed! error=%d\n", hInstance);
    FreeLibrary(hInstance);

    /* check empty string */
    hInstance = LoadLibrary("");
    ok((unsigned long)hInstance == 2, "LoadLibrary(" ") failed! error=%d\n", hInstance);
    FreeLibrary(hInstance);

    /* check weird parameters */
    hInstance = LoadLibrary("notfound");
    ok((unsigned long)hInstance == 2, "LoadLibrary(notfound) failed! error=%d\n", hInstance);
    FreeLibrary(hInstance);

    /* check valid parameters */
    hInstance = LoadLibrary("commdlg");
    ok(hInstance >= HINSTANCE_ERROR, "LoadLibrary(commdlg) failed! error=%d\n",
       hInstance);
    FreeLibrary(hInstance);

    hInstance = LoadLibrary("kernel");  /* core dll */
    ok(hInstance >= HINSTANCE_ERROR, "LoadLibrary(ver) failed! error=%d\n",
       hInstance);
    FreeLibrary(hInstance);
}

/* Why can't we find kernel module under windows XP? */
static void test_Module(void)
{
    HMODULE hMod;
    LOADPARMS lp;
    UINT ar[2];
    LPCSTR modulename = "kernel";
    BOOL ret;

    /* check NULL parameters */
    hMod = LoadModule(NULL, NULL);
    ok((int) hMod == 0, "LoadModule(NULL, NULL) failed!, error=%d\n", hMod);

    /* check empty string */
    hMod = LoadModule("", NULL);
    ok((int) hMod == 2, "LoadModule(\"\", NULL) failed!, error=%d\n", hMod);    /* file not found */

    /* initialize LOADPARMS */
    lp.segEnv = 0;
    lp.lpszCmdLine = NULL;      /* lpszCmdLine cannot be set to NULL */
    ar[0] = 2;
    ar[1] = SW_SHOW;
    lp.lpShow = ar;
    lp.lpReserved = NULL;
    hMod = LoadModule(modulename, &lp);
    /* LoadModule should return error 2 because of lpszCmdLine is set to NULL */
    todo_wine {
        ok((int) hMod == 2,
           "LoadModule with NULL lpszCmdLine failed!, error=%d\n", hMod);
    }

    /* check with valid parameters */
    lp.lpszCmdLine = "";
    hMod = LoadModule(modulename, &lp);
    ok((int) hMod >= 32, "LoadModule(kernel) failed! error=%d\n", hMod);
    ret = FreeModule(hMod);
    ok(ret == TRUE, "FreeModule failed!\n");

    /* 121 chars */
    lp.lpszCmdLine =
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901";
    hMod = LoadModule(modulename, &lp);
    ok((int) hMod >= 32,
       "LoadModule with 121 chars in lpszCmdLine failed! error=%d\n", hMod);
    ret = FreeModule(hMod);
    ok(ret == TRUE, "FreeModule failed! error=%d\n", hMod);
    lp.lpszCmdLine = "";

    /* check lpShow */
    lp.lpShow = NULL;
    hMod = LoadModule(modulename, &lp);
    ok((int) hMod >= 32, "LoadModule with lpShow=Null failed! error=%d\n",
       hMod);
    FreeModule(hMod);
    lp.lpShow = ar;

    ar[0] = 1;
    hMod = LoadModule(modulename, &lp);
    ok((int) hMod >= 32, "LoadModule with ar[0]=1 failed! error=%d\n", hMod);
    ret = FreeModule(hMod);
    ok(ret == TRUE, "FreeModule fail! error=%d\n", ret);
    ar[0] = 2;

    ar[0] = 0xabcd;
    hMod = LoadModule(modulename, &lp);
    ok((int) hMod >= 32, "LoadModule with ar[0]=0xabcd failed! error=%d\n",
       hMod);
    ret = FreeModule(hMod);
    ok(ret == TRUE, "FreeModule fail! error=%d\n", ret);
    ar[0] = 2;

    ar[1] = 0xabcd;
    hMod = LoadModule(modulename, &lp);
    ok((int) hMod >= 32, "LoadModule with ar[1]=0xabcd failed! error=%d\n",
       hMod);
    ret = FreeModule(hMod);
    ok(ret == TRUE, "FreeModule fail! error=%d\n", ret);
    ar[1] = SW_SHOW;

    /* check lpReserved */
    lp.lpReserved = (UINT *) 0x12345678;
    hMod = LoadModule(modulename, &lp);
    ok((int) hMod >= 32, "LoadModule with ar[1]=0xabcd failed! error=%d\n",
       hMod);
    ret = FreeModule(hMod);
    ok(ret == TRUE, "FreeModule fail! error=%d\n", ret);
    lp.lpReserved = NULL;

    /* check empty string */
    hMod = LoadModule("", &lp);
    ok((int) hMod == 2, "LoadModule with an empty string module name failed! error=%d\n", hMod);        /* file not found */

    /* check wrong pointer */
    hMod = LoadModule(NULL, (LPVOID) 0x1234);   /* Wine failure #1 */
    ok((int) hMod == 0, "LoadModule with a NULL module name failed! error=%d\n",
       hMod);

    /* check nonexisting file */
    hMod = LoadModule("c:\\DOESNOTEXIST", &lp);
    ok((int) hMod == 2, "LoadModule with a file that does not exist failed! error=%d\n", hMod); /* file not found */

    /* check strange backslash */
    hMod = LoadModule("C:\\\\command.com", &lp);
    ok((int) hMod == 2, "LoadModule with a strange backslash failed! error=%d\n", hMod);        /* hmm, why error 5 ? */

    /* check strange filename */
    hMod = LoadModule("C:\\command.com\\", &lp);        /* Wine failure #2 */
    ok((int) hMod == 2, "LoadModule with a strange filename failed! error=%d\n", hMod); /* path was not found */

    /* check strange filename */
    hMod = LoadModule("C:\\command.comfoobar", &lp);
    ok((int) hMod == 2, "LoadModule with a strange filename failed! error=%d\n", hMod); /* file not found */
}

static void test_GetProcAddress(void)
{
    LOADPARMS lp;
    HMODULE hMod, hMod2;
    DWORD(FAR PASCAL * GetFileVersionInfoSize) (LPCSTR filename,
                                                DWORD * lpdwHandle);
    int (FAR PASCAL * MessageBox) (HWND hWndParent, LPCSTR lpszMessage,
                                   LPCSTR lpszTitle, UINT uStyle);
    UINT ar[2];

    /* set up lp */
    lp.segEnv = 0;
    lp.lpszCmdLine = "";
    ar[0] = 2;
    ar[1] = SW_SHOW;
    lp.lpShow = ar;
    lp.lpReserved = NULL;

    /* GetFileVersionInfoSizr */
    hMod = LoadModule("ver", &lp);
    GetFileVersionInfoSize = GetProcAddress(hMod, "GetFileVersionInfoSize");
    ok(GetFileVersionInfoSize != NULL,
       "GetProcAddress of GetFileVersionInfoSize failed!\n");

    /* MessageBox */
    hMod2 = LoadModule("user", &lp);
    MessageBox = GetProcAddress(hMod2, "MessageBox");
    ok(MessageBox != NULL, "GetProcessAddress of MessageBox failed!\n");
    FreeLibrary(hMod);
}


START_TEST(sys)
{
    test_GetVersion();
    test_Library();
    test_Module();
    test_GetProcAddress();
}
