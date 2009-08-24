/*
 * Unit tests for dos file functions
 *
 * Copyright (c) 2007 Google, Jennifer Lai
 * Copyright (c) 2004 Markus Amsler
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
 * Compiled with Open WatCOM C16 Optimizing Version 1.6
 */

#include <stdio.h>
#include "dos_func.h"
#include "test.h"


char far *filename = "testfile.xxx";
char far *sillytext =
    "en larvig liten text dx \033 gx hej 84 hej 4484 ! \001\033 bla bl\na.. bla bla."
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "1234 43 4kljf lf &%%%&&&&&& 34 4 34   3############# 33 3 3 3 # 3## 3"
    "sdlkfjasdlkfj a dslkj adsklf  \n  \nasdklf askldfa sdlkf \nsadklf asdklf asdf ";

int max_open_files()
{
    unsigned fhs[255];
    char fn[255];
    int i, ret = 0;
    unsigned b;

    for (i = 0; i < 255; i++) {
        sprintf(fn, "file_%d.xxx", i);
        if (!dos_creat(fn, 0, &fhs[i])) {
            ok((fhs[i] == 0x04), "error should be 0x04 too many open files\n");
            ret = i;
            goto err;
        }
    }
    ret = i;

  err:
    for (i = 0; i < ret; i++) {
        sprintf(fn, "file_%d.xxx", i);
        dos_close(fhs[i], &b);
        dos_unlink(fn, 0, &b);
    }
    return ret;
}

static void test_set_handle_count(void)
{
    unsigned ret;
/*      
 *   FIXME: Wine's SetHandleCount is a stub.
 *   Please see line 1187 in /wine/dlls/kernel32/file.c 
 */
    todo_wine ok(max_open_files() == 15, "max_open_files != 15\n");
    ok(dos_set_handle_count(0, &ret), "dos_set_handle_count(0) failed\n");

    todo_wine ok(max_open_files() == 15, "max_open_files != 15\n");
    ok(dos_set_handle_count(21, &ret), "dos_set_handle_count(21) failed\n");
    todo_wine ok(max_open_files() == 16, "max_open_files != 16\n");
    ok(dos_set_handle_count(22, &ret), "dos_set_handle_count(21) failed\n");
    todo_wine ok(max_open_files() == 17, "max_open_files != 17\n");

/*
 * Default System File Table size on windows is 20.
 * So normally you can only open 17 files (20-3 default system files (stdio,prn.aux).
 * That's why the following test fails on DOS.
 */
/*
    todo_wine ok((temp = dos_set_handle_count(100, &ret)), "dos_set_handle_count(100) failed temp=%d\n", temp);
    todo_wine ok((temp = max_open_files())==95, "max_open_files != 95\n",temp);
*/

    ok(dos_set_handle_count(20, &ret), "dos_set_handle_count(20) failed\n");
    todo_wine ok(max_open_files() == 17, "max_open_files != 17\n");     /*set_handle_count shouldn't shrink*/

    trace("test_set_handle_count();\n");
}

static void test_dos_read(void)
{
    unsigned filehandle;
    char buffer[1000];
    unsigned bytes_read;
    long bytes_wanted;
    long i;
    unsigned ret;
    long seeked;

    dos_unlink(filename, 0, &ret);
    ok(dos_creat(filename, 0, &filehandle), "couldn't create file\n");

    ok(dos_write
       (filehandle, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write complains\n");

    ok(dos_close(filehandle, &ret), "dos_close complains\n");

    if (!dos_open(filename, 0, 0, &filehandle)) {
        trace("couldn't open file errno=%d\n", filehandle);
        return;
    }
    ok(dos_read
       (filehandle, buffer, 2 * strlen((char const *) sillytext),
        &bytes_read), "couldn't read from file\n");

    ok(strlen((char const *) sillytext) == bytes_read,
       "file read size error\n");
    for (bytes_wanted = 0; bytes_wanted < strlen((char const *) sillytext);
         bytes_wanted++) {
        ok(dos_lseek(filehandle, 0, FILE_BEGIN, &seeked), "dos_lseek failed\n");
        ok(seeked == 0, "dos_lseek seeked wrong 0\n");
        ok(dos_read(filehandle, buffer, bytes_wanted, &ret),
           "dos_read failed\n");
        ok(ret == bytes_wanted, "erratic _hread return value\n");
        for (i = 0; i < bytes_wanted; i++) {
            ok(buffer[i] == sillytext[i], "that's not what's written\n");
            ok(buffer[i] == sillytext[i], "n");
        }
    }
    ok(dos_close(filehandle, &ret), "dos_close complains\n");

    ok(dos_unlink(filename, 0, &ret), "dos_unlink failed\n");
    trace("test_dos_read();\n");
}


static void test_dos_open(void)
{
    unsigned filehandle;
    unsigned bytes_read;
    unsigned ret;
    char buffer[1000];

    ok(dos_creat(filename, 0, &filehandle), "couldn't create file\n");

    ok(dos_write
       (filehandle, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write complains\n");

    ok(dos_close(filehandle, &ret), "dos_close complains\n");

    ok(dos_open(filename, OF_READ, 0, &filehandle), "couldn't open file\n");
    ok(!dos_write
       (filehandle, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write shouldn't be able to write!\n");
    ok(dos_read
       (filehandle, buffer, strlen((char const *) sillytext), &bytes_read),
       "dos_read failed\n");
    ok(strlen((char const *) sillytext) == bytes_read,
       "file read size error\n");
    ok(dos_close(filehandle, &ret), "dos_close complains\n");

    ok(dos_open(filename, OF_READWRITE, 0, &filehandle),
       "couldn't open file\n");
    ok(dos_read
       (filehandle, buffer, 2 * strlen((char const *) sillytext),
        &bytes_read), "dos_read failed\n");
    ok(strlen((char const *) sillytext) == bytes_read,
       "file read size error\n");
    ok(dos_write
       (filehandle, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write should write just fine\n");
    ok(dos_close(filehandle, &ret), "dos_close complains\n");

    ok(dos_open(filename, OF_WRITE, 0, &filehandle), "couldn't open file\n");
    ok(!dos_read(filehandle, buffer, 1, &ret),
       "you should only be able to write this file\n");
    ok(dos_write
       (filehandle, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write should write just fine\n");
    ok(dos_close(filehandle, &ret), "dos_close complains\n");

    ok(dos_unlink(filename, 0, &ret), "dos_unlink failed\n");

    ok(dos_open("nul", 0, 0, &filehandle), "couldn't open 'nul' file\n");
    ok(dos_close(filehandle, &ret), "dos_close complains\n");

    ok(dos_open("C:\\nul", 0, 0, &filehandle),
       "couldn't open 'C:\\nul' file\n");
    ok(dos_close(filehandle, &ret), "dos_close complains\n");

    ok(dos_open("C:nul", 0, 0, &filehandle), "couldn't open 'C:\\nul' file\n");
    ok(dos_close(filehandle, &ret), "dos_close complains\n");
    trace("test_dos_open();\n");
    /* TODO - add tests for the SHARE modes  -  use two processes to pull this one off */
}

static void test_dos_dup(void)
{
    unsigned h1, h2;
    unsigned ret;
    long pos;

    ok(dos_creat(filename, 0, &h1), "couldn't create file\n");

    ok(dos_write(h1, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write complains\n");

    ok(dos_dup(h1, &h2), "dos_dup failed\n");

    ok(dos_lseek(h1, 100, FILE_BEGIN, &pos), "dos_lseek failed\n");
    ok(dos_lseek(h2, 0, FILE_CURRENT, &pos), "dos_lseek failed\n");
    ok(pos == 100, "dos_lseek should be bound together\n");

    ok(dos_lseek(h2, 0, FILE_END, &pos), "dos_lseek failed\n");
    ok(dos_write(h2, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write complains\n");
    ok(dos_lseek(h1, 0, FILE_END, &pos), "dos_lseek failed\n");
    ok(pos == 2 * strlen((char const *) sillytext),
       "dos_write should be bound together\n");

    ok(dos_close(h1, &ret), "dos_close complains\n");

    ok(dos_write(h2, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write complains\n");
    ok(dos_close(h2, &ret), "dos_close complains\n");

    ok(dos_unlink(filename, 0, &ret), "dos_unlink failed\n");
    trace("test_dos_dup();\n");
}

static void test_dos_dup2(void)
{
    unsigned h1, h2;
    unsigned ret;
    long pos;

    ok(dos_creat(filename, 0, &h1), "couldn't create file\n");

    ok(dos_write(h1, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write complains\n");

    h2 = 12;
    ok(dos_dup2(h1, h2, &ret), "dos_dup2 failed\n");

    ok(dos_lseek(h1, 100, FILE_BEGIN, &pos), "dos_lseek failed\n");
    ok(dos_lseek(h2, 0, FILE_CURRENT, &pos), "dos_lseek failed\n");
    ok(pos == 100, "dos_lseek should be bound together\n");

    ok(dos_lseek(h2, 0, FILE_END, &pos), "dos_lseek failed\n");
    ok(dos_write(h2, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write complains\n");
    ok(dos_lseek(h1, 0, FILE_END, &pos), "dos_lseek failed\n");
    ok(pos == 2 * strlen((char const *) sillytext),
       "dos_write should be bound together\n");

    ok(dos_close(h1, &ret), "dos_close complains\n");

    ok(dos_write(h2, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write complains\n");
    ok(dos_close(h2, &ret), "dos_close complains\n");

    ok(dos_unlink(filename, 0, &ret), "dos_unlink failed\n");

    /* check IO redirection */
    ok(dos_creat(filename, 0, &h1), "couldn't create file\n");
    ok(dos_dup(1, &h2), "dos_dup failed\n");
    ok(dos_dup2(h1, 1, &ret), "dos_dup2 failed\n");
    ok(dos_write(1, sillytext, strlen((char const *) sillytext), &ret),
       "dos_write complains\n");
    ok(dos_lseek(1, 0, FILE_END, &pos), "dos_lseek failed\n");
    ok(pos == strlen((char const *) sillytext),
       "dos_write should be bound together\n");

    ok(dos_dup2(h2, 1, &ret), "dos_dup2 failed\n");
    ok(dos_close(h2, &ret), "dos_close complains\n");
    ok(dos_close(h1, &ret), "dos_close complains\n");

    ok(dos_unlink(filename, 0, &ret), "dos_unlink failed\n");
    trace("test_dos_dup2();\n");
}

static void test_dos_get_device_info(void)
{
    unsigned h1, h2;
    unsigned ret;
    unsigned flags;

    ok(dos_open("nul", 0, 0, &h1), "couldn't open 'nul' file\n");
    ok(dos_get_device_info(h1, &ret), "dos_get_device info failed\n");
    flags = IOCTL_DEVICE | DEVICE_NUL | DEVICE_EOF;
/*	
 *	Wine's dos_get_device_info does not retrieve the correponding device info.
 *	Please see lines 2788 in /wine/dlls/winedos/int21.c for details.
 *	DX is not set appropriately according to the fd.				
 */
    todo_wine ok((ret & 0xff) == flags, "nul wrong flags\n");

    ok(dos_dup(1, &h2), "dos_dup failed\n");
    ok(dos_dup2(h1, 1, &ret), "dos_dup2 failed\n");
    ok(dos_get_device_info(1, &ret), "dos_get_device info failed\n");

    todo_wine ok((ret & 0xff) == flags, "dup2 wrong flags\n");

    ok(dos_dup2(h2, 1, &ret), "dos_dup2 failed\n");

    ok(dos_close(h2, &ret), "dos_close complains\n");
    ok(dos_close(h1, &ret), "dos_close complains\n");

    flags =
        IOCTL_DEVICE | DEVICE_STDIN | DEVICE_STDOUT | DEVICE_SPECIAL |
        DEVICE_EOF;

    ok(dos_get_device_info(0, &ret), "dos_get_device info failed\n");
    todo_wine ok((ret & 0xff) == flags, "0 wrong flags\n");

    ok(dos_get_device_info(1, &ret), "dos_get_device info failed\n");
    todo_wine ok((ret & 0xff) == flags, "1 wrong flags\n");

    ok(dos_get_device_info(2, &ret), "dos_get_device info failed\n");
    todo_wine ok((ret & 0xff) == flags, "2 wrong flags\n");

    flags = IOCTL_DEVICE | DEVICE_EOF;
    ok(dos_get_device_info(3, &ret), "dos_get_device info failed\n");
    ok((ret & 0xff) == flags, "3 wrong flags\n");

    ok(dos_get_device_info(4, &ret), "dos_get_device info failed\n");
    ok((ret & 0xff) == flags, "4 wrong flags\n");
    trace("test_dos_get_device_info();\n");
}


static void test_dos_close(void)
{
    unsigned filehandle;
    unsigned ret;

    ok(dos_creat(filename, 0, &filehandle), "couldn't create file\n");

    ok(dos_write(filehandle, sillytext, strlen((char const *) sillytext),
                 &ret), "dos_write complains\n");

    ok(dos_close(filehandle, &ret), "dos_close complains\n");

    ok(dos_unlink(filename, 0, &ret), "dos_unlink failed\n");
    trace("test_dos_close();\n");
}


START_TEST(dos_file)
{
    test_set_handle_count();
    test_dos_close();
    test_dos_open();
    test_dos_read();
    test_dos_dup();
    test_dos_dup2();
    test_dos_get_device_info();
}
