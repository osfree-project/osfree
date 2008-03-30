/*
 * Unit tests for win16 file functions
 *
 * Copyright (c) 2002, 2004 Jakob Eriksson
 * Copyright (c) 2004 Markus Amsler
 * Copyright (c) 2007 Google, Jennifer Lai
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * Compiled with Open Watcom C16 Version 1.6 (http://www.openwatcom.org)
 *   > wcl -l=windows file.c
 *
 */

#include <stdio.h>
#include <io.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include "test.h"

#define HFILE_ERROR     ((HFILE)-1)
#define FILE_BEGIN              0

#define LPTR (LMEM_FIXED | LMEM_ZEROINIT)
#define LHND (LMEM_MOVEABLE | LMEM_ZEROINIT)
#define FILE_ATTRIBUTE_NORMAL S_IREAD|S_IWRITE

#define FILE_BEGIN              0
#define FILE_CURRENT            1
#define FILE_END                2

#define INT    int
#define strlen lstrlen

long WINAPI _hread(HFILE, void __huge *, long);
long WINAPI _hwrite(HFILE, const void __huge *, long);
HFILE WINAPI _lcreat(LPCSTR, INT);
HFILE WINAPI _lclose(HFILE);
LONG WINAPI _llseek(HFILE, LONG, INT);
HFILE WINAPI _lopen(LPCSTR, INT);
INT WINAPI lstrlen(LPCSTR);


BOOL WINAPI DeleteFile(LPCSTR str)
{
    int ret;
    ret = unlink((char const *) str);
    if (ret)
	ret = 0;
    else
	ret = -1;
    return ret;
}

BOOL WINAPI SetFileAttributes(LPCSTR file, DWORD attr)
{
    int ret;
    ret = chmod((char const *) file, attr);	
    if (ret)
	ret = 0;
    else
	ret = -1;
    return ret;
}

LPCSTR filename = "testfile.xxx";
LPCSTR sillytext =
    "en larvig liten text dx \033 gx hej 84 hej 4484 ! \001\033 bla bl\na.. bla bla."
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234  4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "sdlkfjasdlkfj a dslkj adsklf  \n  \nasdklf askldfa sdlkf \nsadklf asdklf asdf ";

int max_open_files()
{
    HFILE fhs[255];
    char fn[255];
    int i, ret = 0;

    for (i = 0; i < 255; i++) {
	sprintf(fn, "file_%d.xxx", i);
	fhs[i] = _lcreat(fn, 0);
	if (fhs[i] == HFILE_ERROR) {
	    ret = i;
	    goto err;
	}
    }
	ret = i;
	
  err:
    for (i = 0; i < ret; i++) {
	_lclose(fhs[i]);
	sprintf(fn, "file_%d.xxx", i);
	DeleteFile(fn);
    }
	DeleteFile("file_251.xxx");
    return ret;
}

static void test_SetHandleCount(void)
{

	todo_wine {
/*
 *  FIXME: Wine's SetHandleCount is a stub.
 *  Please see line 1187 in /wine/dlls/kernel32/file.c
 *                                                        */
    ok(max_open_files() == 15, "max_open_files != 15\n");
    ok(SetHandleCount(0) == 20, "SetHandleCount(0) != 20\n");
    ok(SetHandleCount(19) == 20, "SetHandleCount(19) != 20\n");
	}
	ok(SetHandleCount(20) == 20, "SetHandleCount(20)!=20\n");	
    todo_wine ok(max_open_files() == 15, "max_open_files != 15\n");
	ok(SetHandleCount(21) == 21, "SetHandleCount(21) != 21\n");
    todo_wine ok(max_open_files() == 16, "max_open_files != 16\n");
	ok(SetHandleCount(100) == 100, "SetHandleCount(100) != 100\n");
	todo_wine ok(SetHandleCount(33) == 100, "SetHandleCount shouldn't shrink\n");
    todo_wine ok(max_open_files() == 95, "max_open_files != 95\n");
	ok(SetHandleCount(254) == 254, "SetHandleCount(254) != 254\n");
	ok(SetHandleCount(255) == 255, "SetHandleCount(255) != 255\n");
	todo_wine {
	ok(SetHandleCount(256) == 255, "SetHandleCount(256) != 255\n");
	ok(SetHandleCount(257) == 255, "SetHandleCount(257) != 255\n");
	ok(SetHandleCount(20) == 255, "SetHandleCount shouldn't shrink\n");
	}
    trace("test_SetHandleCount();\n");
}


static void test__hread(void)
{
    HFILE filehandle;
    char buffer[1000];
    long bytes_read;
    long bytes_wanted;
    long i;

    SetFileAttributes(filename, FILE_ATTRIBUTE_NORMAL);	/* be sure to remove stale files */
    DeleteFile(filename);
    filehandle = _lcreat(filename, 0);
    if (filehandle == HFILE_ERROR) {
	ok(0, "couldn't create file \"%s\" (err=%ld)\n", filename,
	   GetLastError());
	return;
    }

    ok(HFILE_ERROR != _hwrite(filehandle, sillytext, strlen(sillytext)),
       "_hwrite complains\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    filehandle = _lopen(filename, OF_READ);

    ok(HFILE_ERROR != filehandle,
       "couldn't open file \"%s\" again (err=%ld)\n", filename,
       GetLastError());

    bytes_read = _hread(filehandle, buffer, 2 * strlen(sillytext));

    ok(lstrlen(sillytext) == bytes_read, "file read size error\n");

    for (bytes_wanted = 0; bytes_wanted < lstrlen(sillytext);
	 bytes_wanted++) {
	ok(0 == _llseek(filehandle, 0, FILE_BEGIN), "_llseek complains\n");
	ok(_hread(filehandle, buffer, bytes_wanted) == bytes_wanted,
	   "erratic _hread return value\n");
	for (i = 0; i < bytes_wanted; i++) {
	    ok(buffer[i] == sillytext[i], "that's not what's written\n");
	}
    }

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    ok(DeleteFile(filename) != 0, "DeleteFile failed (%ld)\n",
       GetLastError());
    trace("test__hread();\n");
}


static void test__hwrite(void)
{
    HFILE filehandle;
    char buffer[1000];
    long bytes_read;
    long bytes_written;
    long blocks;
    long i;
    char *contents;
    HLOCAL memory_object;
    char checksum[1];

    filehandle = _lcreat(filename, 0);
    if (filehandle == HFILE_ERROR) {
	ok(0, "couldn't create file \"%s\" (err=%ld)\n", filename,
	   GetLastError());
	return;
    }

    ok(HFILE_ERROR != _hwrite(filehandle, "", 0), "_hwrite complains\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    filehandle = _lopen(filename, OF_READ);

    bytes_read = _hread(filehandle, buffer, 1);

    ok(0 == bytes_read, "file read size error\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    filehandle = _lopen(filename, OF_READWRITE);

    bytes_written = 0;
    checksum[0] = '\0';
    srand((unsigned) time(NULL));
    for (blocks = 0; blocks < 10; blocks++) {
	for (i = 0; i < (long) sizeof(buffer); i++) {
	    buffer[i] = rand();
	    checksum[0] = checksum[0] + buffer[i];
	}
	ok(HFILE_ERROR != _hwrite(filehandle, buffer, sizeof(buffer)),
	   "_hwrite complains\n");
	bytes_written = bytes_written + sizeof(buffer);
    }

    ok(HFILE_ERROR != _hwrite(filehandle, checksum, 1),
       "_hwrite complains\n");
    bytes_written++;

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    memory_object = LocalAlloc(LPTR, bytes_written);

    ok(0 != memory_object,
       "LocalAlloc fails. (Could be out of memory.)\n");

    contents = LocalLock(memory_object);

    filehandle = _lopen(filename, OF_READ);

    contents = LocalLock(memory_object);

    ok(NULL != contents, "LocalLock whines\n");

    bytes_read = _hread(filehandle, contents, bytes_written);
    ok(bytes_written == bytes_read,
       "read length differ from write length\n");

    checksum[0] = '\0';
    i = 0;
    do {
	checksum[0] = checksum[0] + contents[i];
	i++;
    }
    while (i < bytes_written - 1);

    ok(checksum[0] == contents[i],
       "stored checksum differ from computed checksum\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    ok(DeleteFile(filename) != 0, "DeleteFile failed (%ld)\n",
       GetLastError());
    trace("test__hwrite();\n");
}


static void test__lclose(void)
{
    HFILE filehandle;

    filehandle = _lcreat(filename, 0);
    if (filehandle == HFILE_ERROR) {
	ok(0, "couldn't create file \"%s\" (err=%ld)\n", filename,
	   GetLastError());
	return;
    }

    ok(HFILE_ERROR != _hwrite(filehandle, sillytext, strlen(sillytext)),
       "_hwrite complains\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    ok(DeleteFile(filename) != 0, "DeleteFile failed (%ld)\n",
       GetLastError());
    trace("test__lclose();\n");
}


static void test__lcreat(void)
{
    HFILE filehandle;
    char buffer[1000];

    filehandle = _lcreat(filename, 0);
    if (filehandle == HFILE_ERROR) {
	ok(0, "couldn't create file \"%s\" (err=%ld)\n", filename,
	   GetLastError());
	return;
    }

    ok(HFILE_ERROR != _hwrite(filehandle, sillytext, strlen(sillytext)),
       "_hwrite complains\n");

    ok(0 == _llseek(filehandle, 0, FILE_BEGIN), "_llseek complains\n");

    ok(_hread(filehandle, buffer, strlen(sillytext)) == lstrlen(sillytext),
       "erratic _hread return value\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    ok(DeleteFile(filename) != 0, "DeleteFile failed (%ld)\n",
       GetLastError());

    filehandle = _lcreat(filename, 1);	/* readonly */
    ok(HFILE_ERROR != filehandle,
       "couldn't create file \"%s\" (err=%ld)\n", filename,
       GetLastError());

    ok(HFILE_ERROR != _hwrite(filehandle, sillytext, strlen(sillytext)),
       "_hwrite shouldn't be able to write never the less\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    ok(0 == DeleteFile(filename),
       "shouldn't be able to delete a readonly file\n");

    ok(SetFileAttributes(filename, FILE_ATTRIBUTE_NORMAL) != 0,
       "couldn't change attributes on file\n");

    ok(DeleteFile(filename) != 0,
       "now it should be possible to delete the file!\n");

    filehandle = _lcreat(filename, 2);
    ok(HFILE_ERROR != filehandle,
       "couldn't create file \"%s\" (err=%ld)\n", filename,
       GetLastError());

    ok(HFILE_ERROR != _hwrite(filehandle, sillytext, strlen(sillytext)),
       "_hwrite complains\n");

    ok(0 == _llseek(filehandle, 0, FILE_BEGIN), "_llseek complains\n");

    ok(_hread(filehandle, buffer, strlen(sillytext)) == lstrlen(sillytext),
       "erratic _hread return value\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    ok(DeleteFile(filename) != 0, "DeleteFile failed (%ld)\n",
       GetLastError());

    filehandle = _lcreat(filename, 4);	/* SYSTEM file */
    ok(HFILE_ERROR != filehandle,
       "couldn't create file \"%s\" (err=%ld)\n", filename,
       GetLastError());

    ok(HFILE_ERROR != _hwrite(filehandle, sillytext, strlen(sillytext)),
       "_hwrite complains\n");

    ok(0 == _llseek(filehandle, 0, FILE_BEGIN), "_llseek complains\n");

    ok(_hread(filehandle, buffer, strlen(sillytext)) == lstrlen(sillytext),
       "erratic _hread return value\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    ok(DeleteFile(filename) != 0, "DeleteFile failed (%ld)\n",
       GetLastError());
    trace("test__lcreat();\n");
}


static void test__llseek(void)
{
    int i;
    HFILE filehandle;
    char buffer[1];
    long bytes_read;

    filehandle = _lcreat(filename, 0);
    if (filehandle == HFILE_ERROR) {
	ok(0, "couldn't create file \"%s\" (err=%ld)\n", filename,
	   GetLastError());
	return;
    }

    for (i = 0; i < 400; i++) {
	ok(HFILE_ERROR !=
	   _hwrite(filehandle, sillytext, strlen(sillytext)),
	   "_hwrite complains\n");
    }
    ok(HFILE_ERROR !=
       _llseek(filehandle, 400 * strlen(sillytext), FILE_CURRENT),
       "should be able to seek\n");
    ok(HFILE_ERROR !=
       _llseek(filehandle, 27 + 35 * strlen(sillytext), FILE_BEGIN),
       "should be able to seek\n");

    bytes_read = _hread(filehandle, buffer, 1);
    ok(1 == bytes_read, "file read size error\n");
    ok(buffer[0] == sillytext[27], "_llseek error, it got lost seeking\n");
    ok(HFILE_ERROR !=
       _llseek(filehandle, -400 * strlen(sillytext), FILE_END),
       "should be able to seek\n");
    ok(HFILE_ERROR != _llseek(filehandle, 0, FILE_BEGIN),
       "should be able to seek\n");

    bytes_read = _hread(filehandle, buffer, 1);
    ok(1 == bytes_read, "file read size error\n");
    ok(buffer[0] == sillytext[0], "_llseek error, it got lost seeking\n");
    ok(HFILE_ERROR != _llseek(filehandle, 1000000, FILE_END),
       "should be able to seek past file; poor, poor Windows programmers\n");
    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    ok(DeleteFile(filename) != 0, "DeleteFile failed (%ld)\n",
       GetLastError());
    trace("test__llseek();\n");
}


static void test__llopen(void)
{
    HFILE filehandle;
    UINT bytes_read;
    char buffer[1000];

    filehandle = _lcreat(filename, 0);
    if (filehandle == HFILE_ERROR) {
	ok(0, "couldn't create file \"%s\" (err=%ld)\n", filename,
	   GetLastError());
	return;
    }

    ok(HFILE_ERROR != _hwrite(filehandle, sillytext, strlen(sillytext)),
       "_hwrite complains\n");
    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    filehandle = _lopen(filename, OF_READ);
    ok(HFILE_ERROR == _hwrite(filehandle, sillytext, strlen(sillytext)),
       "_hwrite shouldn't be able to write!\n");
    bytes_read = _hread(filehandle, buffer, strlen(sillytext));
    ok(strlen(sillytext) == bytes_read, "file read size error\n");
    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    filehandle = _lopen(filename, OF_READWRITE);
    bytes_read = _hread(filehandle, buffer, 2 * strlen(sillytext));
    ok(strlen(sillytext) == bytes_read, "file read size error\n");
    ok(HFILE_ERROR != _hwrite(filehandle, sillytext, strlen(sillytext)),
       "_hwrite should write just fine\n");
    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    filehandle = _lopen(filename, OF_WRITE);
    ok(HFILE_ERROR == _hread(filehandle, buffer, 1),
       "you should only be able to write this file\n");
    ok(HFILE_ERROR != _hwrite(filehandle, sillytext, strlen(sillytext)),
       "_hwrite should write just fine\n");
    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    ok(DeleteFile(filename) != 0, "DeleteFile failed (%ld)\n",
       GetLastError());
    trace("test__llopen();\n");
    /* TODO - add tests for the SHARE modes  -  use two processes to pull this one off */
}


static void test__lread(void)
{
    HFILE filehandle;
    char buffer[1000];
    long bytes_read;
    UINT bytes_wanted;
    UINT i;

    filehandle = _lcreat(filename, 0);
    if (filehandle == HFILE_ERROR) {
	ok(0, "couldn't create file \"%s\" (err=%ld)\n", filename,
	   GetLastError());
	return;
    }

    ok(HFILE_ERROR != _hwrite(filehandle, sillytext, strlen(sillytext)),
       "_hwrite complains\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    filehandle = _lopen(filename, OF_READ);

    ok(HFILE_ERROR != filehandle,
       "couldn't open file \"%s\" again (err=%ld)\n", filename,
       GetLastError());

    bytes_read = _lread(filehandle, buffer, 2 * strlen(sillytext));

    ok(lstrlen(sillytext) == bytes_read, "file read size error\n");

    for (bytes_wanted = 0; bytes_wanted < strlen(sillytext);
	 bytes_wanted++) {
	ok(0 == _llseek(filehandle, 0, FILE_BEGIN), "_llseek complains\n");
	ok(_lread(filehandle, buffer, bytes_wanted) == bytes_wanted,
	   "erratic _hread return value\n");
	for (i = 0; i < bytes_wanted; i++) {
	    ok(buffer[i] == sillytext[i], "that's not what's written\n");
	}
    }

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    ok(DeleteFile(filename) != 0, "DeleteFile failed (%ld)\n",
       GetLastError());
    trace("test_lread();\n");
}


static void test__lwrite(void)
{
    HFILE filehandle;
    char buffer[1000];
    long bytes_read;
    long bytes_written;
    long blocks;
    long i;
    char *contents;
    HLOCAL memory_object;
    char checksum[1];

    filehandle = _lcreat(filename, 0);
    if (filehandle == HFILE_ERROR) {
	ok(0, "couldn't create file \"%s\" (err=%ld)\n", filename,
	   GetLastError());
	return;
    }

    ok(HFILE_ERROR != _lwrite(filehandle, "", 0), "_hwrite complains\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    filehandle = _lopen(filename, OF_READ);

    bytes_read = _hread(filehandle, buffer, 1);

    ok(0 == bytes_read, "file read size error\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    filehandle = _lopen(filename, OF_READWRITE);

    bytes_written = 0;
    checksum[0] = '\0';
    srand((unsigned) time(NULL));
    for (blocks = 0; blocks < 10; blocks++) {
	for (i = 0; i < (long) sizeof(buffer); i++) {
	    buffer[i] = rand();
	    checksum[0] = checksum[0] + buffer[i];
	}
	ok(HFILE_ERROR != _lwrite(filehandle, buffer, sizeof(buffer)),
	   "_hwrite complains\n");
	bytes_written = bytes_written + sizeof(buffer);
    }

    ok(HFILE_ERROR != _lwrite(filehandle, checksum, 1),
       "_hwrite complains\n");
    bytes_written++;

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    memory_object = LocalAlloc(LPTR, bytes_written);

    ok(0 != memory_object, "LocalAlloc fails, could be out of memory\n");

    contents = LocalLock(memory_object);

    filehandle = _lopen(filename, OF_READ);

    contents = LocalLock(memory_object);

    ok(NULL != contents, "LocalLock whines\n");

    ok(bytes_written == _hread(filehandle, contents, bytes_written),
       "read length differ from write length\n");

    checksum[0] = '\0';
    i = 0;
    do {
	checksum[0] += contents[i];
	i++;
    }
    while (i < bytes_written - 1);

    ok(checksum[0] == contents[i],
       "stored checksum differ from computed checksum\n");

    ok(HFILE_ERROR != _lclose(filehandle), "_lclose complains\n");

    ok(DeleteFile(filename) != 0, "DeleteFile failed (%ld)\n",
       GetLastError());
    trace("test_lwrite();\n");
}


START_TEST(file)
{

    test_SetHandleCount();
    test__lclose();
    test__hread();
    test__hwrite();
    test__lclose();
    test__lcreat();
    test__llseek();
    test__llopen();
    test__lread();
    test__lwrite();

}
