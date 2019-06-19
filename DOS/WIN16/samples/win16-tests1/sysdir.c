
#define STRICT

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "test.h"

static void test_GetSystemDirectory(void)
{
    char buffer[MAX_PATH + 4];
    UINT res;
    UINT total;

    buffer[0] = 0;
    res = GetSystemDirectory(NULL, 0);
    trace("returned %d and '%s'\n", res, buffer);
    /* res includes the terminating Zero */
    ok(res > 0, "returned %d (expected '>0')\n", res);

    total = res;

    res = GetSystemDirectory(NULL, total - 1);
    trace("returned %d and '%s'\n", res, buffer);
    /* 95+NT: total (includes the terminating Zero)
       98+ME: 0 with ERROR_INVALID_PARAMETER */
    ok((res == total) || (!res),
       "returned %d (expected '%d' or: '0' with "
       "ERROR_INVALID_PARAMETER)\n", res, total);

    if (total > MAX_PATH)
	return;

    buffer[0] = '\0';
    res = GetSystemDirectory(buffer, total);
    trace("returned %d and '%s'\n", res, buffer);
    /* res does not include the terminating Zero */
    ok((res == (total - 1)) && (buffer[0]),
       "returned %d and '%s' (expected '%d' and a string)\n",
       res, buffer, total - 1);

    buffer[0] = '\0';
    res = GetSystemDirectory(buffer, total + 1);
    trace("returned %d and '%s'\n", res, buffer);
    /* res does not include the terminating Zero */
    ok((res == (total - 1)) && (buffer[0]),
       "returned %d and '%s' (expected '%d' and a string)\n",
       res, buffer, total - 1);

    memset(buffer, '#', total + 1);
    buffer[total + 2] = '\0';
    res = GetSystemDirectory(buffer, total - 1);
    trace("returned %d and '%s'\n", res, buffer);
    /* res includes the terminating Zero) */
    ok(res == total, "returned %d and '%s' (expected '%d')\n",
       res, buffer, total);

    memset(buffer, '#', total + 1);
    buffer[total + 2] = '\0';
    res = GetSystemDirectory(buffer, total - 2);
    trace("returned %d and '%s'\n", res, buffer);
    /* res includes the terminating Zero) */
    ok(res == total, "returned %d and '%s' (expected '%d')\n",
       res, buffer, total);
}

static void test_GetWindowsDirectory(void)
{
    char buffer[MAX_PATH + 4];
    UINT res;
    UINT total;

    buffer[0] = 0;
    res = GetWindowsDirectory(NULL, 0);
    trace("returned %d and '%s'\n", res, buffer);
    /* res includes the terminating Zero */
    ok(res > 0, "returned %d (expected '>0')\n", res);

    total = res;

    res = GetWindowsDirectory(NULL, total - 1);
    trace("returned %d and '%s'\n", res, buffer);
    /* 95+NT: total (includes the terminating Zero)
       98+ME: 0 with ERROR_INVALID_PARAMETER */
    ok((res == total) || (!res),
       "returned %d (expected '%d' or: '0' with "
       "ERROR_INVALID_PARAMETER)\n", res, total);

    if (total > MAX_PATH)
	return;

    buffer[0] = '\0';
    res = GetWindowsDirectory(buffer, total);
    trace("returned %d and '%s'\n", res, buffer);
    /* res does not include the terminating Zero */
    ok((res == (total - 1)) && (buffer[0]),
       "returned %d and '%s' (expected '%d' and a string)\n",
       res, buffer, total - 1);

    buffer[0] = '\0';
    res = GetWindowsDirectory(buffer, total + 1);
    trace("returned %d and '%s'\n", res, buffer);
    /* res does not include the terminating Zero */
    ok((res == (total - 1)) && (buffer[0]),
       "returned %d and '%s' (expected '%d' and a string)\n",
       res, buffer, total - 1);

    memset(buffer, '#', total + 1);
    buffer[total + 2] = '\0';
    res = GetWindowsDirectory(buffer, total - 1);
    trace("returned %d and '%s'\n", res, buffer);
    /* res includes the terminating Zero) */
    ok(res == total, "returned %d and '%s' (expected '%d')\n",
       res, buffer, total);

    memset(buffer, '#', total + 1);
    buffer[total + 2] = '\0';
    res = GetWindowsDirectory(buffer, total - 2);
    trace("returned %d and '%s'\n", res, buffer);
    /* res includes the terminating Zero) */
    ok(res == total, "returned %d and '%s' (expected '%d')\n",
       res, buffer, total);
}

START_TEST(sysdir)
{
    test_GetSystemDirectory();
    test_GetWindowsDirectory();
}
