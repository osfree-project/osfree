/*
 * Win16 Unit test suite for memory allocation functions.
 *
 * Copyright 2007 Jennfier Lai, Google
 * Modified from /wine-git/dlls/kernel32/tests/alloc.c written by
 * Geoffrey Hausheer, Copyright 2002
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

#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include "test.h"

#define NO_ERROR 0

static void test_Global(void)
{
    DWORD memchunk;
    HGLOBAL mem1, mem2, mem2a;
    char FAR *mem2ptr;
    long mem2_long;
    DWORD mem2b;
    UINT error, i;
    memchunk = 50000;

    SetLastError(NO_ERROR);
    /* Check that a normal alloc works */
    mem1 = GlobalAlloc(0, memchunk);
    ok(mem1, "GlobalAlloc failed\n");
    if (mem1) {
        ok(GlobalSize(mem1) >= memchunk,
           "GlobalAlloc should return a big enough memory block\n");
    }
    /* Check that a 'zeroing' alloc works */
    mem2 = GlobalAlloc(GMEM_ZEROINIT, memchunk);
    ok(mem2, "GlobalAlloc failed: error=%d\n", GetLastError());
    if (mem2) {
        ok(GlobalSize(mem2) >= memchunk,
           "GlobalAlloc should return a big enough memory block\n");
        mem2ptr = GlobalLock(mem2);
        mem2_long = mem2 & 0xffff;
        mem2_long = mem2_long << 16;
        ok(mem2_long == mem2ptr,
           "GlobalLock should have returned the same memory as was allocated\n");
        if (mem2ptr) {
            error = 0;
            for (i = 0; i < memchunk; i++) {
                if (mem2ptr[i] != 0) {
                    error = 1;
                }
            }
            ok(!error,
               "GlobalAlloc should have zeroed out it's allocated memory\n");
        }
    }
    /* Check that GlobalReAlloc works */
    /* Check that we can change GMEM_FIXED to GMEM_MOVEABLE */
    mem2a = GlobalReAlloc(mem2, 0, GMEM_MODIFY | GMEM_MOVEABLE);
    if (mem2a) {
        mem2 = mem2a;
        mem2ptr = GlobalLock(mem2a);
        GlobalUnlock(mem2a);
        ok(mem2ptr != NULL && !GlobalUnlock(mem2a)
           && GetLastError() == NO_ERROR,
           "Converting from FIXED to MOVEABLE didn't REALLY work\n");
    }
    /* Check that ReAllocing memory works as expected */
    mem2a = GlobalReAlloc(mem2, 2 * memchunk, GMEM_MOVEABLE | GMEM_ZEROINIT);
    ok(mem2a != NULL, "GlobalReAlloc failed\n");
    if (mem2a) {
        ok(GlobalSize(mem2a) >= 2 * memchunk, "GlobalReAlloc failed\n");
        mem2ptr = GlobalLock(mem2a);
        ok(mem2ptr != NULL, "GlobalLock Failed\n");
        if (mem2ptr) {
            error = 0;
            for (i = 0; i < memchunk; i++) {
                if (mem2ptr[memchunk + i] != 0) {
                    error = 1;
                }
            }
            ok(!error,
               "GlobalReAlloc should have zeroed out it's allocated memory\n");

            /* Check that GlobalHandle works */
            mem2b = GlobalHandle((unsigned long) mem2a) & 0xffff;
            ok(mem2b == mem2a,
               "GlobalHandle didn't return the correct memory handle\n");

            /* Check that we can't discard locked memory */
            mem2b = GlobalDiscard(mem2a);
            if (mem2b == NULL) {
                GlobalUnlock(mem2a);
                ok(!GlobalUnlock(mem2a)
                   && GetLastError() == NO_ERROR, "GlobalUnlock Failed\n");
            }
        }
    }
    if (mem1) {
        ok(GlobalFree(mem1) == NULL, "GlobalFree failed\n");
    }
    if (mem2a) {
        ok(GlobalFree(mem2a) == NULL, "GlobalFree failed\n");
    } else {
        ok(GlobalFree(mem2) == NULL, "GlobalFree failed\n");
    }
}

static void test_Local(void)
{
    DWORD memchunk;
    HLOCAL mem1, mem2, mem2a, mem2b;
    char FAR *mem2ptr;
    long mem2_long;
    UINT error, i;
    memchunk = 5000;

    /* Check that a normal alloc works */
    mem1 = LocalAlloc(0, memchunk);
    ok(mem1 != NULL, "LocalAlloc failed: error=%d\n", GetLastError());
    if (mem1) {
        ok(LocalSize(mem1) >= memchunk,
           "LocalAlloc should return a big enough memory block\n");
    }

    /* Check that a 'zeroing' and lock alloc works */
    mem2 = LocalAlloc(LMEM_ZEROINIT | LMEM_MOVEABLE, memchunk);
    ok(mem2 != NULL, "LocalAlloc failed: error=%d\n", GetLastError());
    if (mem2) {
        ok(LocalSize(mem2) >= memchunk,
           "LocalAlloc should return a big enough memory block\n");
        mem2ptr = LocalLock(mem2);
        mem2_long = mem2_long << 16;
        ok(mem2ptr != NULL, "LocalLock: error=%d\n", GetLastError());
        if (mem2ptr) {
            error = 0;
            for (i = 0; i < memchunk; i++) {
                if (mem2ptr[i] != 0) {
                    error = 1;
                }
            }
            ok(!error,
               "LocalAlloc should have zeroed out it's allocated memory\n");
            SetLastError(0);
            error = LocalUnlock(mem2);
            ok(error == 0 && GetLastError() == NO_ERROR,
               "LocalUnlock Failed: rc=%d err=%d\n", error, GetLastError());
        }
    }
    mem2a = LocalFree(mem2);
    ok(mem2a == NULL, "LocalFree failed: %p\n", mem2a);

    /* Reallocate mem2 as moveable memory */
    mem2 = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, memchunk);
    ok(mem2 != NULL, "LocalAlloc failed to create moveable memory, error=%d\n",
       GetLastError());

    /* Check that ReAllocing memory works as expected */
    mem2a = LocalReAlloc(mem2, 2 * memchunk, LMEM_MOVEABLE | LMEM_ZEROINIT);
    ok(mem2a != NULL, "LocalReAlloc failed, error=%d\n", GetLastError());
    if (mem2a) {
        ok(LocalSize(mem2a) >= 2 * memchunk, "LocalReAlloc failed\n");
        mem2ptr = LocalLock(mem2a);
        ok(mem2ptr != NULL, "LocalLock Failed\n");
        if (mem2ptr) {
            error = 0;
            for (i = 0; i < memchunk; i++) {
                if (mem2ptr[memchunk + i] != 0) {
                    error = 1;
                }
            }
            ok(!error,
               "LocalReAlloc should have zeroed out it's allocated memory\n");


            /* Check that LocalHandle works */
            mem2b = LocalHandle((unsigned long) mem2ptr) & 0xffff;
            ok(mem2b == mem2a,
               "LocalHandle didn't return the correct memory handle\n");
            /* Check that we can't discard locked memory */
            mem2b = LocalDiscard(mem2a);
            ok(mem2b == NULL, "Discarded memory we shouldn't have\n");
            SetLastError(NO_ERROR);
            ok(!LocalUnlock(mem2a)
               && GetLastError() == NO_ERROR, "LocalUnlock Failed\n");
        }
    }
    if (mem1) {
        ok(LocalFree(mem1) == NULL, "LocalFree failed\n");
    }
    if (mem2a) {
        ok(LocalFree(mem2a) == NULL, "LocalFree failed\n");
    } else {
        ok(LocalFree(mem2) == NULL, "LocalFree failed\n");
    }
}

START_TEST(alloc)
{
    test_Global();
    test_Local();
}
