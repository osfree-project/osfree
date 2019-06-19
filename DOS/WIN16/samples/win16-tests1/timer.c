/*
 * Win16 Unit test suite for timer functions.
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
#include <stdio.h>
#include <windows.h>
#include <dos.h>    /*for delay function */
#include "test.h"

#define TIMER_ID  0x19

struct timer_info {
    HWND hWnd;
    DWORD id;
};

static VOID CALLBACK tfunc(HWND hwnd, UINT uMsg, UINT id, DWORD dwTime)
{
}

static void test_timers(void)
{
    struct timer_info info[10];
    int i;

    /*Create a timer with specified ID */
    info[0].hWnd = CreateWindow("STATIC", NULL,
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, 0,
                                NULL, NULL, 0);

    info[0].id = SetTimer(info[0].hWnd, TIMER_ID, 10000, tfunc);
    ok(info[0].id, "SetTimer failed\n");
    ok(info[0].id == TIMER_ID,
       "SetTimer timer id different, got %d, should be %d\n", info[0].id,
       TIMER_ID);

    ok(KillTimer(info[0].hWnd, TIMER_ID), "KillTimer failed\n");

    ok(DestroyWindow(info[0].hWnd), "failed to destroy window\n");

    /*Create a timer with NULL ID */
    for (i = 0; i < 10; i++) {
        info[i].id = SetTimer(NULL, 0, 5000, tfunc);
        ok(info[i].id, "SetTimer failed\n");
        ok(KillTimer(NULL, info[i].id), "KillTimer failed\n");
    }
}

static void test_GetCurrentTime(void)
{
    DWORD time1, time2, time3;
    time1 = GetCurrentTime();
    delay(20);
    time2 = GetTickCount();
    delay(20);
    time3 = GetCurrentTime();
    ok(time1, "GetCurrenTime 1 failed!\n");
    ok(time2, "GetCurrenTime 2 failed!\n");
    ok(time3, "GetCurrenTime 3 failed!\n");
    ok(time2 > time1, "Time count shouldn't shrink! time1=%ld, time2=%ld\n",
       time1, time2);
    ok((time3 > time2)
       && (time3 > time1), "Time count shouldn't shrink! time3=%ld\n", time3);
    printf("Current session has elapsed appx %ld ms!\n", GetCurrentTime());
}

START_TEST(timer)
{
    test_timers();
    test_GetCurrentTime();
}
