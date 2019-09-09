/*
 * Win16 Unit test suite for atom functions.
 *
 * Copyright 2007 Jennfier Lai, Google
 * Modified from /wine-git/dlls/kernel32/tests/atom.c written by
 * Alexandre Julliard, Copyright (c) 2002
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


static void do_init(char *tmp, const char *pattern, int len)
{
    const char *p = pattern;

    while (len--) {
        *tmp++ = *p++;
        if (!*p)
            p = pattern;
    }
    *tmp = '\0';
}

static void test_add_atom(void)
{
    ATOM atom;
    LONG i;

    SetLastError(0xdeadbeef);
    atom = GlobalAddAtom("foobar");
    ok(atom >= 0xc000, "bad atom id %x\n", atom);
    ok(GetLastError() == 0xdeadbeef, "GlobalAddAtomA set last error\n");

    /* Verify that it can be found (or not) appropriately */
    ok(GlobalFindAtom("foobar") == atom, "could not find atom foobar\n");
    ok(GlobalFindAtom("FOOBAR") == atom, "could not find atom FOOBAR\n");
    ok(!GlobalFindAtom("_foobar"), "found _foobar\n");

    /* Test integer atoms
     * (0x0001 .. 0xbfff) should be valid;
     * (0xc000 .. 0xffff) should be invalid */
    SetLastError(0xdeadbeef);
    ok(GlobalAddAtom(0) == 0
       && GetLastError() == 0xdeadbeef, "succeeded to add atom 0\n");

    for (i = 1; i <= 0xbfff; i++) {
        SetLastError(0xdeadbeef);
        ok(GlobalAddAtom((LPCSTR) i) == i && GetLastError() == 0xdeadbeef,
           "failed to add atom %x\n", i);
    }
    for (i = 0xc000; i <= 0xffff; i++) {
        ok(!GlobalAddAtom((LPCSTR) i), "succeeded adding %x\n", i);
    }
}

static void test_get_atom_name(void)
{
    char buf[10];
    LONG i;
    char in[257], out[257];
    UINT len;

    ATOM atom = GlobalAddAtom("foobar");

    /* Get the name of the atom we added above */
    memset(buf, '.', sizeof(buf));
    len = GlobalGetAtomName(atom, buf, 10);
    ok(len == strlen("foobar"), "bad length %d\n", len);
    ok(!memcmp(buf, "foobar\0...", 10), "bad buffer contents\n");

    /* Check error code returns */
    memset(buf, '.', 10);
    ok(!GlobalGetAtomName(atom, buf, 0), "succeeded\n");
    ok(!memcmp(buf, "..........", 10), "should not touch buffer\n");

    /* Test integer atoms */
    for (i = 0; i <= 0xbfff; i++) {
        memset(buf, 'a', 10);
        len = GlobalGetAtomName((ATOM) i, buf, 10);
        if (i) {
            char res[20];
            ok((len > 1) && (len < 7), "bad length %d\n", len);
            sprintf(res, "#%ld", i);
            memset(res + strlen(res) + 1, 'a', 10);
            ok(!memcmp(res, buf, 10), "bad buffer contents %s, i=%x\n", buf, i);
        } else
            ok(!len, "bad length %d\n", len);
        len = GlobalGetAtomName((ATOM) i, buf, 2);
    }
    memset(buf, '.', sizeof(buf));
    len = GlobalGetAtomName(atom, buf, 6);
    ok(len == 0, "bad length %d\n", len);
    ok(!memcmp(buf, "fooba\0....", 10), "bad buffer contents\n");

    /* test string limits & overflow */
    do_init(in, "abcdefghij", 255);
    atom = GlobalAddAtom(in);
    ok(atom, "couldn't add atom for %s\n", in);
    len = GlobalGetAtomName(atom, out, sizeof(out));
    ok(len == 255, "length mismatch (%u instead of 255)\n", len);
    for (i = 0; i < 255; i++) {
        ok(out[i] == "abcdefghij"[i % 10],
           "wrong string at %i (%c instead of %c)\n", i, out[i],
           "abcdefghij"[i % 10]);
    }
    ok(out[255] == '\0', "wrong end of string\n");
    memset(out, '.', sizeof(out));
    SetLastError(0xdeadbeef);
    len = GlobalGetAtomName(atom, out, 10);

    for (i = 0; i < 9; i++) {
        ok(out[i] == "abcdefghij"[i % 10],
           "wrong string at %i (%c instead of %c)\n", i, out[i],
           "abcdefghij"[i % 10]);
    }
    ok(out[9] == '\0', "wrong end of string\n");
    ok(out[10] == '.', "wrote after end of buf\n");
    do_init(in, "abcdefghij", 256);
    atom = GlobalAddAtom(in);
    ok(!atom, "succeeded\n");
}

static void test_error_handling(void)
{
    char buffer[260];

    memset(buffer, 'a', 256);
    buffer[256] = 0;
    ok(!GlobalAddAtom(buffer), "add succeeded\n");
    ok(!GlobalFindAtom(buffer), "find succeeded\n");
}

static void test_local_add_atom(void)
{
    ATOM atom;
    LONG i;

    SetLastError(0xdeadbeef);
    atom = AddAtom("foobar");
    ok(atom >= 0xc000, "bad atom id %x\n", atom);
    ok(GetLastError() == 0xdeadbeef, "AddAtom set last error\n");

    /* Verify that it can be found (or not) appropriately */
    ok(FindAtom("foobar") == atom, "could not find atom foobar\n");
    ok(FindAtom("FOOBAR") == atom, "could not find atom FOOBAR\n");
    ok(!FindAtom("_foobar"), "found _foobar\n");

    /* Test integer atoms
     ** (0x0001 .. 0xbfff) should be valid;
     ** (0xc000 .. 0xffff) should be invalid */
    SetLastError(0xdeadbeef);
    ok(AddAtom(0) == 0
       && GetLastError() == 0xdeadbeef, "succeeded to add atom 0\n");

    SetLastError(0xdeadbeef);
    for (i = 1; i <= 0xbfff; i++) {
        SetLastError(0xdeadbeef);
        ok(AddAtom((LPCSTR) i) == i && GetLastError() == 0xdeadbeef,
           "failed to add atom %x\n", i);
    }

    for (i = 0xc000; i <= 0xffff; i++) {
        ok(!AddAtom((LPCSTR) i), "succeeded adding %x\n", i);
    }
}

static void test_local_get_atom_name(void)
{
    char buf[10], in[257], out[257];
    LONG i;
    UINT len;
    ATOM atom = AddAtom("foobar");

    /* Get the name of the atom we added above */
    memset(buf, '.', sizeof(buf));
    len = GetAtomName(atom, buf, 10);
    ok(len == strlen("foobar"), "bad length %d\n", len);
    ok(!memcmp(buf, "foobar\0...", 10), "bad buffer contents\n");

    /* Get the name of the atom we added above */
    memset(buf, '.', sizeof(buf));
    len = GetAtomName(atom, buf, 6);
    ok(len == 5, "bad length %d\n", len);
    ok(!memcmp(buf, "fooba\0....", 10), "bad buffer contents\n");

    /* Check error code returns */
    memset(buf, '.', 10);
    ok(!GetAtomName(atom, buf, 0), "succeeded\n");
    ok(!memcmp(buf, "..........", 10), "should not touch buffer\n");

    /* Test integer atoms */
    for (i = 0; i <= 0xbfff; i++) {
        memset(buf, 'a', 10);
        len = GetAtomName((ATOM) i, buf, 10);
        if (i) {
            char res[20];
            ok((len > 1) && (len < 7), "bad length %d for %s\n", len, buf);
            sprintf(res, "#%ld", i);
            memset(res + strlen(res) + 1, 'a', 10);
            ok(!memcmp(res, buf, 10), "bad buffer contents %s\n", buf);
        } else {
            todo_wine ok(!len, "bad length %d\n", len);
        }
        len = GetAtomName((ATOM) i, buf, 1);
        ok(!len, "succeed with %u for %u\n", len, i);
    }

    /* test string limits & overflow */
    do_init(in, "abcdefghij", 255);
    atom = AddAtom(in);
    ok(atom, "couldn't add atom for %s\n", in);
    len = GetAtomName(atom, out, sizeof(out));
    ok(len == 255, "length mismatch (%u instead of 255)\n", len);
    for (i = 0; i < 255; i++) {
        ok(out[i] == "abcdefghij"[i % 10],
           "wrong string at %i (%c instead of %c)\n", i, out[i],
           "abcdefghij"[i % 10]);
    }
    ok(out[255] == '\0', "wrong end of string\n");
    memset(out, '.', sizeof(out));
    len = GetAtomName(atom, out, 10);
    ok(len == 9, "succeeded %d\n", len);
    for (i = 0; i < 9; i++) {
        ok(out[i] == "abcdefghij"[i % 10],
           "wrong string at %i (%c instead of %c)\n", i, out[i],
           "abcdefghij"[i % 10]);
    }
    ok(out[9] == '\0', "wrong end of string\n");
    ok(out[10] == '.', "buffer overwrite\n");
    do_init(in, "abcdefghijk", 256);
    atom = AddAtom(in);
    todo_wine ok(!atom, "succeeded\n");
}

static void test_local_error_handling(void)
{
    char buffer[260];

    memset(buffer, 'a', 256);
    buffer[256] = 0;
    todo_wine {
        ok(!AddAtom(buffer), "add succeeded\n");
        ok(!FindAtom(buffer), "find succeeded\n");
    }
}


START_TEST(atom)
{
    test_add_atom();
    test_get_atom_name();
    test_error_handling();
    test_local_add_atom();
    test_local_get_atom_name();
    test_local_error_handling();
}
