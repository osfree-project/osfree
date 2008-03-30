/*
 * Win16 Unit tests for profile functions
 *
 * Copyright 2007 Google, Jennifer Lai
 * Modified by /wine-git/dlls/kernel32/tests/profile.c
 * Copyright (c) 2003 Stefan Leichter
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
#include <windows.h>
#include <win16.h>
#include <unistd.h>
    
#include "test.h"
    
#define KEY      "ProfileInt"
#define SECTION  "Test"
#define TESTFILE ".\\profile1.ini"
#define TESTFILE2 ".\\profile2.ini"
#define TESTFILE3 ".\\profile3.ini"
    struct _profileInt {
    LPCSTR section;
    LPCSTR key;
    LPCSTR value;
    LPCSTR iniFile;
    int defaultVal;
     UINT result;
     UINT result9x;
};
BOOL WINAPI DeleteFile(LPCSTR str) 
{
    int ret;
    ret = unlink((char const *) str);
    if (ret)
        ret = 0;
    ret = -1;
    return ret;
}
static void test_profile_int(void) 
{
    struct _profileInt profileInt[] = { {NULL, NULL, NULL, NULL, 70, 0, 0}, /*  0 */ 
    {NULL, NULL, NULL, TESTFILE, -1, 4294967295U, 0}, 
        {NULL, NULL, NULL, TESTFILE, 1, 1, 0}, 
        {SECTION, NULL, NULL, TESTFILE, -1, 4294967295U, 0}, 
        {SECTION, NULL, NULL, TESTFILE, 1, 1, 0}, 
        {NULL, KEY, NULL, TESTFILE, -1, 4294967295U, 0}, /*  5 */ 
    {NULL, KEY, NULL, TESTFILE, 1, 1, 0}, 
        {SECTION, KEY, NULL, TESTFILE, -1, 4294967295U, 4294967295U}, 
        {SECTION, KEY, NULL, TESTFILE, 1, 1, 1}, 
        {SECTION, KEY, "-1", TESTFILE, -1, 4294967295U, 4294967295U}, 
        {SECTION, KEY, "-1", TESTFILE, 1, 4294967295U, 4294967295U}, /* 10 */ 
    {SECTION, KEY, "1", TESTFILE, -1, 1, 1}, 
        {SECTION, KEY, "1", TESTFILE, 1, 1, 1}, 
        {SECTION, KEY, "+1", TESTFILE, -1, 1, 0}, 
        {SECTION, KEY, "+1", TESTFILE, 1, 1, 0}, 
        {SECTION, KEY, "4294967296", TESTFILE, -1, 0, 0}, /* 15 */ 
    {SECTION, KEY, "4294967296", TESTFILE, 1, 0, 0}, 
        {SECTION, KEY, "4294967297", TESTFILE, -1, 1, 1}, 
        {SECTION, KEY, "4294967297", TESTFILE, 1, 1, 1}, 
        {SECTION, KEY, "-4294967297", TESTFILE, -1, 4294967295U, 4294967295U}, 
        {SECTION, KEY, "-4294967297", TESTFILE, 1, 4294967295U, 4294967295U},
        /* 20 */ 
    {SECTION, KEY, "42A94967297", TESTFILE, -1, 42, 42}, {SECTION, KEY,
                                                           "42A94967297",
                                                           TESTFILE, 1, 42, 42},
        {SECTION, KEY, "B4294967297", TESTFILE, -1, 0, 0}, {SECTION, KEY,
                                                              "B4294967297",
                                                              TESTFILE, 1, 0,
                                                              0}, 
    };
    int i, num_test = (sizeof(profileInt) / sizeof(struct _profileInt));
    UINT res;
    DeleteFile(TESTFILE);
    for (i = 0; i < num_test; i++) {
        if (profileInt[i].value)
            WritePrivateProfileString(SECTION, KEY, profileInt[i].value,
                                       profileInt[i].iniFile);
        res =
            GetPrivateProfileInt(profileInt[i].section, profileInt[i].key,
                                 profileInt[i].defaultVal,
                                 profileInt[i].iniFile);
        ok((res == profileInt[i].result)
            || (res == profileInt[i].result9x),
            "test<%02d>: ret<%010u> exp<%010u><%010u>\n", i, res,
            profileInt[i].result, profileInt[i].result9x);
    }
    DeleteFile(TESTFILE);
}
static void test_profile_string(void) 
{
    int ret;
    char buf[100];
    char *p;
    char content[] =
        "\r\n[s]\r\nname1=val1\r\nname2=\r\nname3\r\nname4=val4\r\n";
    FILE * fp;
    DeleteFile(TESTFILE2);
    DeleteFile(TESTFILE3);
    
	/* test WritePrivateProfileString */	
    memset(buf, 0, sizeof(buf));
    WritePrivateProfileString("NewSection", "key1", "value1", TESTFILE3);
    WritePrivateProfileString("NewSection", "key2", "value2", TESTFILE3);
    WritePrivateProfileString("NewSection", "key3", "value3", TESTFILE3);

    ret = GetPrivateProfileString("NewSection", NULL, "", buf, sizeof(buf), TESTFILE3);
    for (p = buf + strlen(buf) + 1; *p; p += strlen(p) + 1)
        p[-1] = ',';
    /* and test*/ 
    ok(ret == 15 
       && !strcmp(buf, "key1,key2,key3"),
       "wrong keys returned(%d): %s\n", ret, buf);
    DeleteFile(TESTFILE3);
	
    /* now test that lines WITHOUT an '=' will not be enumerated
       in the case below, name2 is a key while name3 is not. */ 
    ok((fp = fopen(TESTFILE2, "a+")) != NULL, "openfile failed!\n");
    if (fp)
         {
        ret = fwrite(content, sizeof(char), sizeof(content), fp);
        ok(ret > 0, "fwrite failed! \n");
        }
    fclose(fp);
    
    /* enumerate the keys */ 
    memset(buf, 0, sizeof(buf));
    ret = GetPrivateProfileString("s", NULL, "", buf, sizeof(buf), TESTFILE2);
    for (p = buf + strlen(buf) + 1; *p; p += strlen(p) + 1)
        p[-1] = ',';
    
    /* and test : 
	 * BUG: it shouldn't enumerate name3 because of the missing = */ 
	/* will fail with unpatched wine */
    ok(ret == 18
       && !strcmp(buf, "name1,name2,name4"),
       "wrong keys returned(%d): %s\n", ret, buf);
    
    /* add a new key to test that the file is quite usable */ 
    WritePrivateProfileString("s", "name5", "val5", TESTFILE2);
    ret = GetPrivateProfileString("s", NULL, "", buf, sizeof(buf), TESTFILE2);
    for (p = buf + strlen(buf) + 1; *p; p += strlen(p) + 1)
        p[-1] = ',';
	/* will fail with unpatched wine */
    ok(ret == 24
        && !strcmp(buf, "name1,name2,name4,name5"),
        "wrong keys returned(%d): %s\n", ret, buf);
    DeleteFile(TESTFILE2);
}

START_TEST(profile) 
{
    test_profile_int();
    test_profile_string();
}

